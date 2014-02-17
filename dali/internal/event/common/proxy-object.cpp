//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/internal/event/common/proxy-object.h>

// EXTERNAL INCLUDES
#include <algorithm>

// INTERNAL INCLUDES
#include <dali/internal/event/common/stage-impl.h>
#include <dali/internal/update/common/animatable-property.h>
#include <dali/internal/update/animation/scene-graph-constraint-base.h>
#include <dali/internal/update/common/property-owner-messages.h>
#include <dali/internal/event/animation/active-constraint-base.h>
#include <dali/internal/event/animation/constraint-impl.h>
#include <dali/internal/event/common/property-notification-impl.h>
#include <dali/internal/event/common/property-index-ranges.h>

using Dali::Internal::SceneGraph::AnimatableProperty;
using Dali::Internal::SceneGraph::PropertyBase;

namespace Dali
{

namespace Internal
{

namespace // unnamed namespace
{
const int SUPPORTED_CAPABILITIES = Dali::Handle::DYNAMIC_PROPERTIES;  // ProxyObject provides this capability
typedef Dali::Vector<ProxyObject::Observer*>::Iterator ObserverIter;
typedef Dali::Vector<ProxyObject::Observer*>::ConstIterator ConstObserverIter;

static std::string EMPTY_PROPERTY_NAME;
} // unnamed namespace

const int INVALID_PROPERTY_COMPONENT_INDEX = -1;

ProxyObject::ProxyObject()
: mNextCustomPropertyIndex( 0u ),
  mCustomProperties( NULL ),
  mConstraints( NULL ),
  mRemovedConstraints( NULL ),
  mPropertyNotifications( NULL )
{
}

void ProxyObject::AddObserver(Observer& observer)
{
  // make sure an observer doesn't observe the same object twice
  // otherwise it will get multiple calls to OnSceneObjectAdd(), OnSceneObjectRemove() and ProxyDestroyed()
  DALI_ASSERT_DEBUG( mObservers.End() == std::find( mObservers.Begin(), mObservers.End(), &observer));

  mObservers.PushBack( &observer );
}

void ProxyObject::RemoveObserver(Observer& observer)
{
  // Find the observer...
  const ConstObserverIter endIter =  mObservers.End();
  for( ObserverIter iter = mObservers.Begin(); iter != endIter; ++iter)
  {
    if( (*iter) == &observer)
    {
      mObservers.Erase( iter );
      break;
    }
  }
  DALI_ASSERT_DEBUG(endIter != mObservers.End());
}

void ProxyObject::OnSceneObjectAdd()
{
  // Notification for observers
  for( ConstObserverIter iter = mObservers.Begin(),  endIter =  mObservers.End(); iter != endIter; ++iter)
  {
    (*iter)->SceneObjectAdded(*this);
  }

  // enable property notifications in scene graph
  EnablePropertyNotifications();
}

void ProxyObject::OnSceneObjectRemove()
{
  // Notification for observers
  for( ConstObserverIter iter = mObservers.Begin(), endIter = mObservers.End(); iter != endIter; ++iter )
  {
    (*iter)->SceneObjectRemoved(*this);
  }

  // disable property notifications in scene graph
  DisablePropertyNotifications();
}

int ProxyObject::GetPropertyComponentIndex( Property::Index index ) const
{
  return INVALID_PROPERTY_COMPONENT_INDEX;
}

bool ProxyObject::Supports( Capability capability ) const
{
  return (capability & SUPPORTED_CAPABILITIES);
}

unsigned int ProxyObject::GetPropertyCount() const
{
  unsigned int count = GetDefaultPropertyCount();
  if( mCustomProperties )
  {
    count += mCustomProperties->size();
  }
  return count;
}

const std::string& ProxyObject::GetPropertyName( Property::Index index ) const
{
  DALI_ASSERT_ALWAYS( index > Property::INVALID_INDEX && "Property index out of bounds" );

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    return GetDefaultPropertyName( index );
  }

  if( mCustomProperties )
  {
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Property index is invalid" );

    return entry->second.name;
  }
  return EMPTY_PROPERTY_NAME;
}

Property::Index ProxyObject::GetPropertyIndex(const std::string& name) const
{
  Property::Index index = Property::INVALID_INDEX;

  index = GetDefaultPropertyIndex( name );

  if (Property::INVALID_INDEX == index)
  {
    if( mCustomProperties )
    {
      // This is slow, but we're not (supposed to be) using property names frequently
      for ( CustomPropertyLookup::const_iterator iter = mCustomProperties->begin(); mCustomProperties->end() != iter; ++iter )
      {
        if (iter->second.name == name)
        {
          index = iter->first;
          break;
        }
      }
    }
  }

  return index;
}

bool ProxyObject::IsPropertyWritable( Property::Index index ) const
{
  DALI_ASSERT_ALWAYS(index > Property::INVALID_INDEX && "Property index is out of bounds");

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    return IsDefaultPropertyWritable( index );
  }

  if( mCustomProperties)
  {
    // Check that the index is valid
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find property index" );

    return entry->second.IsWritable();
  }
  return false;
}

bool ProxyObject::IsPropertyAnimatable( Property::Index index ) const
{
  DALI_ASSERT_ALWAYS(index > Property::INVALID_INDEX && "Property index is out of bounds");

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    return IsDefaultPropertyAnimatable( index );
  }

  if( mCustomProperties )
  {
    // Check custom property
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find property index" );

    return entry->second.IsAnimatable();
  }
  return false;
}

Property::Type ProxyObject::GetPropertyType( Property::Index index ) const
{
  DALI_ASSERT_ALWAYS(index > Property::INVALID_INDEX && "Property index is out of bounds" );

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    return GetDefaultPropertyType( index );
  }

  if( mCustomProperties )
  {
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find Property index" );

    return entry->second.type;
  }
  return Property::NONE;
}

void ProxyObject::SetProperty( Property::Index index, const Property::Value& propertyValue )
{
  DALI_ASSERT_ALWAYS(index > Property::INVALID_INDEX && "Property index is out of bounds" );

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    DALI_ASSERT_ALWAYS( IsDefaultPropertyWritable(index) && "Property is read-only" );

    SetDefaultProperty( index, propertyValue );
  }
  else if( mCustomProperties )
  {
    CustomPropertyLookup::iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find property index" );
    DALI_ASSERT_ALWAYS( entry->second.IsWritable() && "Property is read-only" );

    // this is only relevant for non animatable properties
    if(entry->second.IsWritable())
    {
      entry->second.value = propertyValue;
    }

    SetCustomProperty(index, entry->second, propertyValue);
  }
}

Property::Value ProxyObject::GetProperty(Property::Index index) const
{
  DALI_ASSERT_ALWAYS( index > Property::INVALID_INDEX && "Property index is out of bounds" );

  Property::Value value;

  if ( index < DEFAULT_PROPERTY_MAX_COUNT )
  {
    value = GetDefaultProperty( index );
  }
  else if( mCustomProperties )
  {
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find property index" );

    if( !entry->second.IsAnimatable() )
    {
      value = entry->second.value;
    }
    else
    {
      BufferIndex bufferIndex( Stage::GetCurrent()->GetEventBufferIndex() );

      switch ( entry->second.type )
      {
        case Property::BOOLEAN:
        {
          AnimatableProperty<bool>* property = dynamic_cast< AnimatableProperty<bool>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::FLOAT:
        {
          AnimatableProperty<float>* property = dynamic_cast< AnimatableProperty<float>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::VECTOR2:
        {
          AnimatableProperty<Vector2>* property = dynamic_cast< AnimatableProperty<Vector2>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::VECTOR3:
        {
          AnimatableProperty<Vector3>* property = dynamic_cast< AnimatableProperty<Vector3>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::VECTOR4:
        {
          AnimatableProperty<Vector4>* property = dynamic_cast< AnimatableProperty<Vector4>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::MATRIX:
        {
          AnimatableProperty<Matrix>* property = dynamic_cast< AnimatableProperty<Matrix>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::MATRIX3:
        {
          AnimatableProperty<Matrix3>* property = dynamic_cast< AnimatableProperty<Matrix3>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        case Property::ROTATION:
        {
          AnimatableProperty<Quaternion>* property = dynamic_cast< AnimatableProperty<Quaternion>* >( entry->second.GetSceneGraphProperty() );
          DALI_ASSERT_DEBUG( NULL != property );

          value = (*property)[ bufferIndex ];
          break;
        }

        default:
        {
          DALI_ASSERT_ALWAYS( false && "PropertyType enumeration is out of bounds" );
          break;
        }
      } // switch(type)
    } // if animatable

  } // if custom

  return value;
}

Property::Index ProxyObject::RegisterProperty( std::string name, const Property::Value& propertyValue)
{
  // Assert that property name is unused
  DALI_ASSERT_ALWAYS( Property::INVALID_INDEX == GetPropertyIndex(name) && "Property index is out of bounds" );

  // Create a new property
  std::auto_ptr<PropertyBase> newProperty;

  switch ( propertyValue.GetType() )
  {
    case Property::BOOLEAN:
    {
      newProperty.reset(new AnimatableProperty<bool>( propertyValue.Get<bool>()));
      break;
    }

    case Property::FLOAT:
    {
      newProperty.reset(new AnimatableProperty<float>( propertyValue.Get<float>()));
      break;
    }

    case Property::VECTOR2:
    {
      newProperty.reset(new AnimatableProperty<Vector2>( propertyValue.Get<Vector2>()));
      break;
    }

    case Property::VECTOR3:
    {
      newProperty.reset(new AnimatableProperty<Vector3>( propertyValue.Get<Vector3>()));
      break;
    }

    case Property::VECTOR4:
    {
      newProperty.reset(new AnimatableProperty<Vector4>( propertyValue.Get<Vector4>()));
      break;
    }

    case Property::MATRIX:
    {
      newProperty.reset(new AnimatableProperty<Matrix>( propertyValue.Get<Matrix>()));
      break;
    }

    case Property::MATRIX3:
    {
      newProperty.reset(new AnimatableProperty<Matrix3>( propertyValue.Get<Matrix3>()));
      break;
    }

    case Property::ROTATION:
    {
      newProperty.reset(new AnimatableProperty<Quaternion>( propertyValue.Get<Quaternion>()));
      break;
    }

    default:
    {
      DALI_LOG_WARNING( "Property Type %d\n", propertyValue.GetType() );
      DALI_ASSERT_ALWAYS( false && "PropertyType enumeration is out of bounds" );
      break;
    }
  }

  // Default properties start from index zero
  if ( 0u == mNextCustomPropertyIndex )
  {
    mNextCustomPropertyIndex = DEFAULT_PROPERTY_MAX_COUNT;
  }

  // Add entry to the property lookup
  const Property::Index index = mNextCustomPropertyIndex++;

  CustomPropertyLookup::const_iterator entry = GetCustomPropertyLookup().find( index );
  DALI_ASSERT_ALWAYS( mCustomProperties->end() == entry && "Custom property already registered" );

  (*mCustomProperties)[ index ] = CustomProperty( name, propertyValue.GetType(), newProperty.get() );

  // The derived class now passes ownership of this new property to a scene-object
  InstallSceneObjectProperty( *(newProperty.release()), name, index );

  return index;
}

Property::Index ProxyObject::RegisterProperty( std::string name, const Property::Value& propertyValue, Property::AccessMode accessMode)
{
  Property::Index index = Property::INVALID_INDEX;

  if(Property::ANIMATABLE == accessMode)
  {
    index = RegisterProperty(name, propertyValue);
  }
  else
  {
    // Default properties start from index zero
    if ( 0u == mNextCustomPropertyIndex )
    {
      mNextCustomPropertyIndex = DEFAULT_PROPERTY_MAX_COUNT;
    }

    // Add entry to the property lookup
    index = mNextCustomPropertyIndex++;
    GetCustomPropertyLookup()[ index ] = CustomProperty( name, propertyValue, accessMode );
  }

  return index;
}

Dali::PropertyNotification ProxyObject::AddPropertyNotification(Property::Index index,
                                                                const Dali::PropertyCondition& condition)
{
  if ( ( index >= DEFAULT_PROPERTY_MAX_COUNT )&&( mCustomProperties ) )
  {
    CustomPropertyLookup::const_iterator entry = mCustomProperties->find( index );
    DALI_ASSERT_ALWAYS( mCustomProperties->end() != entry && "Cannot find property index" );

    DALI_ASSERT_ALWAYS( entry->second.IsAnimatable() && "Property notification added to non animatable property (currently not suppported )");
  }

  Dali::Handle self(this);
  Property target( self, index );

  PropertyNotificationPtr internal = PropertyNotification::New( target, condition );
  Dali::PropertyNotification propertyNotification(internal.Get());

  if( !mPropertyNotifications )
  {
    mPropertyNotifications = new PropertyNotificationContainer;
  }
  mPropertyNotifications->push_back(propertyNotification);

  return propertyNotification;
}

// deprecated
Dali::PropertyNotification ProxyObject::AddPropertyNotification(Property::Index index,
                                                                const Dali::PropertyCondition& condition,
                                                                PropertyNotifyCallbackType callback)
{
  return AddPropertyNotification(index, condition);
}

void ProxyObject::RemovePropertyNotification(Dali::PropertyNotification propertyNotification)
{
  if( mPropertyNotifications )
  {
    PropertyNotificationContainerIter iter = mPropertyNotifications->begin();
    while(iter != mPropertyNotifications->end() )
    {
      if(*iter == propertyNotification)
      {
        mPropertyNotifications->erase(iter);
        // As we can't ensure all references are removed, we can just disable
        // the notification.
        GetImplementation(propertyNotification).Disable();
        return;
      }
      ++iter;
    }
  }
}

void ProxyObject::RemovePropertyNotifications()
{
  if( mPropertyNotifications )
  {
    PropertyNotificationContainerIter iter = mPropertyNotifications->begin();
    while(iter != mPropertyNotifications->end() )
    {
      // As we can't ensure all references are removed, we can just disable
      // the notification.
      GetImplementation(*iter).Disable();
      ++iter;
    }

    mPropertyNotifications->clear();
  }
}

void ProxyObject::EnablePropertyNotifications()
{
  if( mPropertyNotifications )
  {
    PropertyNotificationContainerIter iter = mPropertyNotifications->begin();
    PropertyNotificationContainerIter endIter = mPropertyNotifications->end();

    for( ; iter != endIter; ++iter )
    {
      GetImplementation(*iter).Enable();
    }
  }
}

void ProxyObject::DisablePropertyNotifications()
{
  if( mPropertyNotifications )
  {
    PropertyNotificationContainerIter iter = mPropertyNotifications->begin();
    PropertyNotificationContainerIter endIter = mPropertyNotifications->end();

    for( ; iter != endIter; ++iter )
    {
      GetImplementation(*iter).Disable();
    }
  }
}

Dali::ActiveConstraint ProxyObject::ApplyConstraint( Constraint& constraint )
{
  return Dali::ActiveConstraint(DoApplyConstraint( constraint, NULL/*callback is optional*/ ));
}

Dali::ActiveConstraint ProxyObject::ApplyConstraint( Constraint& constraint, ActiveConstraintCallbackType callback )
{
  return Dali::ActiveConstraint(DoApplyConstraint( constraint, &callback ));
}

ActiveConstraintBase* ProxyObject::DoApplyConstraint( Constraint& constraint, ActiveConstraintCallbackType* callback )
{
  ActiveConstraintBase* activeConstraintImpl = constraint.CreateActiveConstraint();
  DALI_ASSERT_DEBUG( NULL != activeConstraintImpl );

  Dali::ActiveConstraint activeConstraint( activeConstraintImpl );

  if( !mConstraints )
  {
    mConstraints = new ActiveConstraintContainer;
  }
  mConstraints->push_back( activeConstraint );

  activeConstraintImpl->FirstApply( *this, constraint.GetApplyTime(), callback );

  return activeConstraintImpl;
}

void ProxyObject::DeleteRemovedConstraints()
{
  if( ! mRemovedConstraints )
  {
    return;
  }

  // Discard constraints which are fully removed
  for ( ActiveConstraintIter iter = mRemovedConstraints->begin(); mRemovedConstraints->end() != iter ;)
  {
    if ( !( GetImplementation( *iter ).IsRemoving() ) )
    {
      iter = mRemovedConstraints->erase( iter );
    }
    else
    {
      ++iter;
    }
  }
 }

CustomPropertyLookup& ProxyObject::GetCustomPropertyLookup() const
{
  // lazy create
  if( !mCustomProperties )
  {
    mCustomProperties = new CustomPropertyLookup;
  }
  return *mCustomProperties;
}

void ProxyObject::RemoveConstraint( Dali::ActiveConstraint activeConstraint )
{
  if( mConstraints )
  {
    // If we have nothing in the scene-graph, just remove the activeConstraint from container
    const SceneGraph::PropertyOwner* propertyOwner = GetSceneObject();
    if ( NULL == propertyOwner )
    {
      ActiveConstraintIter it( std::find( mConstraints->begin(), mConstraints->end(), activeConstraint ) );
      if( it != mConstraints->end() )
      {
        mConstraints->erase( it );
      }
      delete mRemovedConstraints;
      mRemovedConstraints = NULL;
      return;
    }

    // Discard constraints which are fully removed
    DeleteRemovedConstraints();

    ActiveConstraintIter it( std::find( mConstraints->begin(), mConstraints->end(), activeConstraint ) );
    if( it != mConstraints->end() )
    {
      ActiveConstraintBase& constraint = GetImplementation( *it );

      constraint.BeginRemove();
      mConstraints->erase( it );

      if ( constraint.IsRemoving() )
      {
        if( !mRemovedConstraints )
        {
          mRemovedConstraints = new ActiveConstraintContainer;
        }
        // Wait for remove animation before destroying active-constraints
        mRemovedConstraints->push_back( *it );
      }
    }
  }
}

void ProxyObject::RemoveConstraints()
{
  if( mConstraints )
  {
    // If we have nothing in the scene-graph, just clear constraint containers
    const SceneGraph::PropertyOwner* propertyOwner = GetSceneObject();
    if ( NULL == propertyOwner )
    {
      delete mConstraints;
      mConstraints = NULL;
      delete mRemovedConstraints;
      mRemovedConstraints = NULL;
      return;
    }

    // Discard constraints which are fully removed
    DeleteRemovedConstraints();

    const ActiveConstraintConstIter endIter = mConstraints->end();
    for ( ActiveConstraintIter iter = mConstraints->begin(); endIter != iter; ++iter )
    {
      ActiveConstraintBase& constraint = GetImplementation( *iter );

      constraint.BeginRemove();

      if ( constraint.IsRemoving() )
      {
        if( !mRemovedConstraints )
        {
          mRemovedConstraints = new ActiveConstraintContainer;
        }
        // Wait for remove animation before destroying active-constraints
        mRemovedConstraints->push_back( *iter );
      }
    }

    delete mConstraints;
    mConstraints = NULL;
  }
}

ProxyObject::~ProxyObject()
{
  // Notification for observers
  for( ConstObserverIter iter = mObservers.Begin(), endIter =  mObservers.End(); iter != endIter; ++iter)
  {
    (*iter)->ProxyDestroyed(*this);
  }

  delete mCustomProperties;
  delete mConstraints;
  delete mRemovedConstraints;
  delete mPropertyNotifications;
}

} // namespace Internal

} // namespace Dali