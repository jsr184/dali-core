/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dali/public-api/dali-core.h>
#include <dali-test-suite-utils.h>
#include <test-native-image.h>

using namespace Dali;

#include <mesh-builder.h>

void texture_set_startup(void)
{
  test_return_value = TET_UNDEF;
}

void texture_set_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliTextureNew01(void)
{
  TestApplication application;

  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  DALI_TEST_CHECK( texture );
  END_TEST;
}

int UtcDaliTextureNew02(void)
{
  TestApplication application;
  Texture texture;
  DALI_TEST_CHECK( !texture );
  END_TEST;
}

int UtcDaliTextureNew03(void)
{
  TestApplication application;

  // Create a native image source.
  TestNativeImageNoExtPointer testNativeImage = TestNativeImageNoExt::New( 64u, 64u );

  // Create a texture from the native image source.
  Texture nativeTexture = Texture::New( *testNativeImage );

  // Check the texture was created OK.
  DALI_TEST_CHECK( nativeTexture );

  END_TEST;
}

int UtcDaliTextureCopyConstructor(void)
{
  TestApplication application;

  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  Texture textureCopy(texture);

  DALI_TEST_CHECK( textureCopy );

  END_TEST;
}

int UtcDaliTextureAssignmentOperator(void)
{
  TestApplication application;
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  Texture texture2;
  DALI_TEST_CHECK( !texture2 );

  texture2 = texture;
  DALI_TEST_CHECK( texture2 );

  END_TEST;
}

int UtcDaliTextureDownCast01(void)
{
  TestApplication application;
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  BaseHandle handle(texture);
  Texture texture2 = Texture::DownCast(handle);
  DALI_TEST_CHECK( texture2 );

  END_TEST;
}

int UtcDaliTextureDownCast02(void)
{
  TestApplication application;

  Handle handle = Handle::New(); // Create a custom object
  Texture texture = Texture::DownCast(handle);
  DALI_TEST_CHECK( !texture );
  END_TEST;
}

int UtcDaliTextureUpload01(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //TexImage2D should be called with a null pointer to reserve storage for the texture in the gpu
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  //Upload data to the texture
  callStack.Reset();

  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelData );
  application.SendNotification();
  application.Render();

  //TexImage2D should be called to upload the data
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  //Upload part of the texture
  callStack.Reset();
  bufferSize =  width * height * 2;
  buffer = reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelDataSubImage = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelDataSubImage, 0u, 0u, width/2, height/2, width/2, height/2 );
  application.SendNotification();
  application.Render();

  //TexSubImage2D should be called to upload the data
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width/2 << ", " <<  height/2 << ", " << width/2 << ", " <<  height/2;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexSubImage2D", out.str().c_str() ) );
  }


  END_TEST;
}

int UtcDaliTextureUpload02(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_CUBE, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //TexImage2D should be called six times with a null pointer to reserve storage for the six textures of the cube map
  for( unsigned int i(0); i<6; ++i )
  {
    std::stringstream out;
    out << GL_TEXTURE_CUBE_MAP_POSITIVE_X + i <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );

  //Upload data to the POSITIVE_X face of the texture
  {
    callStack.Reset();

    texture.Upload( pixelData, CubeMapLayer::POSITIVE_X, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the POSITIVE_X face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_POSITIVE_X <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  //Upload data to the NEGATIVE_X face of the texture
  {
    callStack.Reset();

    texture.Upload( pixelData, CubeMapLayer::NEGATIVE_X, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the NEGATIVE_X face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_NEGATIVE_X <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  //Upload data to the POSITIVE_Y face of the texture
  {
    callStack.Reset();
    texture.Upload( pixelData, CubeMapLayer::POSITIVE_Y, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the POSITIVE_Y face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_POSITIVE_Y <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  //Upload data to the NEGATIVE_Y face of the texture
  {
    callStack.Reset();
    texture.Upload( pixelData, CubeMapLayer::NEGATIVE_Y, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the NEGATIVE_Y face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_NEGATIVE_Y <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  //Upload data to the POSITIVE_Z face of the texture
  {
    callStack.Reset();
    texture.Upload( pixelData, CubeMapLayer::POSITIVE_Z, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the POSITIVE_Z face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_POSITIVE_Z <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  //Upload data to the NEGATIVE_Z face of the texture
  {
    callStack.Reset();
    texture.Upload( pixelData, CubeMapLayer::NEGATIVE_Z, 0u, 0u, 0u, width, height );
    application.SendNotification();
    application.Render();

    //TexImage2D should be called to upload the data to the NEGATIVE_Z face
    {
      std::stringstream out;
      out << GL_TEXTURE_CUBE_MAP_NEGATIVE_Z <<", "<< 0u << ", " << width <<", "<< height;
      DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
    }
  }

  END_TEST;
}

int UtcDaliTextureUpload03(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  unsigned int widthMipmap1(32);
  unsigned int heightMipmap1(32);

  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //TexImage2D should be called with a null pointer to reserve storage for the texture in the gpu
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  //Upload data to the texture mipmap 0 and mipmap 1
  callStack.Reset();

  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc(  bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelData, 0u, 0u, 0u, 0u, width, height );

  bufferSize = widthMipmap1 * heightMipmap1 * 4;
  buffer = reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelDataMipmap1 = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelDataMipmap1, 0u, 1u, 0u, 0u, widthMipmap1, heightMipmap1 );
  application.SendNotification();
  application.Render();

  //TexImage2D should be called to upload the data to mipmaps 0 and 1
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 1u << ", " << widthMipmap1 <<", "<< heightMipmap1;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  END_TEST;
}

int UtcDaliTextureUpload04(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  unsigned int widthMipmap1(32);
  unsigned int heightMipmap1(32);

  Texture texture = Texture::New( TextureType::TEXTURE_CUBE, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);
  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //Upload data to the NEGATIVE_X face mipmap 0 and mipmap 1
  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelData, CubeMapLayer::NEGATIVE_X, 0u, 0u, 0u, width, height );

  bufferSize = widthMipmap1 * heightMipmap1 * 4;
  buffer = reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelDataMipmap1 = PixelData::New( buffer, bufferSize, width, height, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelDataMipmap1, CubeMapLayer::NEGATIVE_X, 1u, 0u, 0u, widthMipmap1, heightMipmap1 );
  application.SendNotification();
  application.Render();

  //TexImage2D should be called to upload the data to mipmaps 0 and 1
  {
    std::stringstream out;
    out << GL_TEXTURE_CUBE_MAP_NEGATIVE_X <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }
  {
    std::stringstream out;
    out << GL_TEXTURE_CUBE_MAP_NEGATIVE_X <<", "<< 1u << ", " << widthMipmap1 <<", "<< heightMipmap1;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  END_TEST;
}

int UtcDaliTextureUpload05(void)
{
  TestApplication application;

  //Create a texture with a compressed format
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //CompressedTexImage2D should be called with a null pointer to reserve storage for the texture in the gpu
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("CompressedTexImage2D", out.str().c_str() ) );
  }

  //Upload data to the texture
  callStack.Reset();

  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR, PixelData::FREE );
  texture.Upload( pixelData );
  application.SendNotification();
  application.Render();

  //CompressedTexImage2D should be called to upload the data
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("CompressedTexImage2D", out.str().c_str() ) );
  }

  //Upload part of the texture
  callStack.Reset();
  bufferSize =  width * height * 2;
  buffer = reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelDataSubImage = PixelData::New( buffer, bufferSize, width, height, Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR, PixelData::FREE );
  texture.Upload( pixelDataSubImage, 0u, 0u, width/2, height/2, width/2, height/2 );
  application.SendNotification();
  application.Render();

  //CompressedTexSubImage2D should be called to upload the data
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width/2 << ", " <<  height/2 << ", " << width/2 << ", " <<  height/2;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("CompressedTexSubImage2D", out.str().c_str() ) );
  }


  END_TEST;
}

int UtcDaliTextureUpload06(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  tet_infoline( "Creating a Texure with an alpha channel" );
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  tet_infoline( "TexImage2D should be called with a null pointer to reserve storage for the texture in the gpu" );
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  tet_infoline( "Upload data to the texture" );
  callStack.Reset();

  tet_infoline( "Creating a RGB pixel buffer and adding that to the texture to ensure it is handled correctly" );
  unsigned int bufferSize( width * height * 3 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width, height, Pixel::RGB888, PixelData::FREE );
  texture.Upload( pixelData );
  application.SendNotification();
  application.Render();

  tet_infoline( "TexImage2D should be called to upload the data" );
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("TexImage2D", out.str().c_str() ) );
  }

  END_TEST;
}

int UtcDaliTextureUploadSmallerThanSize(void)
{
  TestApplication application;

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );

  application.GetGlAbstraction().EnableTextureCallTrace(true);

  application.SendNotification();
  application.Render();

  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();

  //TexImage2D should be called with a null pointer to reserve storage for the texture in the gpu
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << width <<", "<< height;
    std::string params;
    DALI_TEST_CHECK( callStack.FindMethodAndGetParameters("TexImage2D", params ) );
    DALI_TEST_EQUALS( out.str(), params, TEST_LOCATION );
  }

  //Upload data to the texture
  callStack.Reset();

  unsigned int bufferSize( width * height * 4 );
  unsigned char* buffer= reinterpret_cast<unsigned char*>( malloc( bufferSize ) );
  PixelData pixelData = PixelData::New( buffer, bufferSize, width/2, height/2, Pixel::RGBA8888, PixelData::FREE );
  texture.Upload( pixelData );
  application.SendNotification();
  application.Render();

  //TexImage2D should be called to upload the data
  {
    std::stringstream out;
    out << GL_TEXTURE_2D <<", "<< 0u << ", " << 0u << ", " <<  0u << ", " << width/2 << ", " <<  height/2;
    std::string params;
    DALI_TEST_CHECK( callStack.FindMethodAndGetParameters("TexSubImage2D", params ) );
    DALI_TEST_EQUALS( out.str(), params, TEST_LOCATION );
  }

  END_TEST;
}

int UtcDaliTextureGenerateMipmaps(void)
{
  TestApplication application;
  unsigned int width(64);
  unsigned int height(64);

  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );
  texture.GenerateMipmaps();

  Texture textureCubemap = Texture::New( TextureType::TEXTURE_CUBE, Pixel::RGBA8888, width, height );
  textureCubemap.GenerateMipmaps();

  application.GetGlAbstraction().EnableTextureCallTrace(true);
  TraceCallStack& callStack = application.GetGlAbstraction().GetTextureTrace();
  application.SendNotification();
  application.Render();

  {
    std::stringstream out;
    out << GL_TEXTURE_2D;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("GenerateMipmap", out.str().c_str() ) );
  }
  {
    std::stringstream out;
    out << GL_TEXTURE_CUBE_MAP;
    DALI_TEST_CHECK( callStack.FindMethodAndParams("GenerateMipmap", out.str().c_str() ) );
  }

  END_TEST;
}

int UtcDaliTextureGetWidth(void)
{
  TestApplication application;
  unsigned int width(64);
  unsigned int height(64);

  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );
  DALI_TEST_EQUALS( texture.GetWidth(), width, TEST_LOCATION );
  END_TEST;
}

int UtcDaliTextureGetHeight(void)
{
  TestApplication application;
  unsigned int width(64);
  unsigned int height(64);

  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );
  DALI_TEST_EQUALS( texture.GetHeight(), height, TEST_LOCATION );

  END_TEST;
}

int UtcDaliTextureContextLoss(void)
{
  tet_infoline("UtcDaliTextureContextLoss\n");
  TestApplication application; // Default config: DALI_DISCARDS_ALL_DATA

  //Create the texture
  unsigned int width(64);
  unsigned int height(64);
  Texture texture = Texture::New( TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height );
  DALI_TEST_CHECK( texture );

  application.SendNotification();
  application.Render(16);

  // Lose & regain context (in render 'thread')
  application.ResetContext();
  DALI_TEST_CHECK( texture );

  END_TEST;
}

int UtcDaliNativeImageTexture(void)
{
  TestApplication application;
  tet_infoline( "UtcDaliNativeImageTexture" );

  TestNativeImagePointer imageInterface = TestNativeImage::New( 16, 16 );
  Texture texture = Texture::New( *(imageInterface.Get()) );
  DALI_TEST_CHECK( texture );

  application.SendNotification();
  application.Render(16);

  DALI_TEST_CHECK( texture );

  END_TEST;
}

