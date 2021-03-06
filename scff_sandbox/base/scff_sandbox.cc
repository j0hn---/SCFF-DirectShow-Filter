﻿// Copyright 2012-2013 Alalf <alalf.iQLc_at_gmail.com>
//
// This file is part of SCFF-DirectShow-Filter(SCFF DSF).
//
// SCFF DSF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCFF DSF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SCFF DSF.  If not, see <http://www.gnu.org/licenses/>.

#include "base/scff_sandbox.h"

#include <libavfilter/drawutils.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3d11.h>

void TestFFDraw() {
  FFDrawContext* test_context = new FFDrawContext;
  FFDrawColor* test_color = new FFDrawColor;
  uint8_t test_fill_color[4] = {255, 255, 255, 255};

  ff_draw_init(test_context, AV_PIX_FMT_RGB0, 0);
  ff_draw_color(test_context, test_color, test_fill_color);

  printf("Hello, World! %d\n", test_context->format);
  getchar();

  delete test_context;
}

namespace {
const D3D_DRIVER_TYPE kDriverTypes[] = {
  D3D_DRIVER_TYPE_HARDWARE,
  D3D_DRIVER_TYPE_WARP,
  D3D_DRIVER_TYPE_REFERENCE
};
const int kDriverTypesCount =
    sizeof(kDriverTypes) / sizeof(kDriverTypes[0]);
D3D_FEATURE_LEVEL kFeatureLevels[] = {
  D3D_FEATURE_LEVEL_11_0,
  D3D_FEATURE_LEVEL_10_1,
  D3D_FEATURE_LEVEL_10_0,
  D3D_FEATURE_LEVEL_9_1
};
const int kFeatureLevelsCount =
    sizeof(kFeatureLevels) / sizeof(kFeatureLevels[0]);
}

void SaveBitmapFile(const byte* bitmap, const int width, const int height, LPCWSTR filename) {
  const int bmp_size = width * height * 4;

  BITMAPINFOHEADER bmp_header;
  RtlZeroMemory(&bmp_header, sizeof(bmp_header));
  bmp_header.biSize = sizeof(bmp_header);
  bmp_header.biWidth = width;
  bmp_header.biHeight = -1 * height;
  bmp_header.biPlanes = 1;
  bmp_header.biBitCount = 32;
  bmp_header.biCompression = BI_RGB;
  bmp_header.biSizeImage = bmp_size;

  BITMAPFILEHEADER bmp_file_header;
  RtlZeroMemory(&bmp_file_header, sizeof(bmp_file_header));
  bmp_file_header.bfType = 0x4D42;
  bmp_file_header.bfSize = sizeof(bmp_file_header) + sizeof(bmp_header) + bmp_size;
  bmp_file_header.bfOffBits = sizeof(bmp_file_header) + sizeof(bmp_header);

  auto bmp_file = CreateFile(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  DWORD written_byte;
  WriteFile(bmp_file, &bmp_file_header, sizeof(bmp_file_header), &written_byte, nullptr);
  WriteFile(bmp_file, &bmp_header, sizeof(bmp_header), &written_byte, nullptr);
  WriteFile(bmp_file, bitmap, bmp_size, &written_byte, nullptr);
  CloseHandle(bmp_file);
}

void TestDXGIDesktopDuplication() {
  // 使いまわし用HRESULT
  HRESULT result;

  // D3D11Device, D3D11DeviceContextの初期化
  // D3D11デバイスとはDirectX11の機能が使える論理的な何かで
  // 現在使っているビデオカードやモニタの情報は無い
  // DXGIのレイヤより一つ上のレイヤにあると思えば良い
  ID3D11Device *device = nullptr;
  ID3D11DeviceContext *device_context = nullptr;
  D3D_FEATURE_LEVEL device_feature_level;
  for each (auto x in kDriverTypes) {
    result = D3D11CreateDevice(
        nullptr,
        x,
        nullptr,
        0,
        kFeatureLevels,
        kFeatureLevelsCount,
        D3D11_SDK_VERSION,
        &device,
        &device_feature_level,
        &device_context);

    if (SUCCEEDED(result)) break;
  }
  if (FAILED(result)) {
    printf("Error @ D3D11CreateDevice\n");
    goto RELEASE;
  }

  // D3D11DeviceをローレベルなDXGIDeviceに変換
  IDXGIDevice *dxgi_device = nullptr;
  result = device->QueryInterface(__uuidof(IDXGIDevice),
      reinterpret_cast<void**>(&dxgi_device));
  if (FAILED(result)) {
    printf("Error @ Query DXGIDevice\n");
    goto RELEASE;
  }

  // DXGIAdapter = ビデオカードの取得
  // (DXGIAdapterはDXGIDeviceの一つ上にある)
  IDXGIAdapter *dxgi_adapter = nullptr;
  result = dxgi_device->GetParent(__uuidof(IDXGIAdapter),
      reinterpret_cast<void**>(&dxgi_adapter));
  dxgi_device->Release();
  dxgi_device = nullptr;
  if (FAILED(result)) {
    printf("Error @ Query DXGIAdapter\n");
    goto RELEASE;
  }
  
  // DXGIOutput = 出力先(モニタ)の取得
  // モニタの数だけあることに注意。コレはマルチモニタ対応がめんどくさいことになりそうだ。
  IDXGIOutput *dxgi_output = nullptr;
  const int primary_output = 0;
  result = dxgi_adapter->EnumOutputs(primary_output, &dxgi_output);
  dxgi_adapter->Release();
  dxgi_adapter = nullptr;
  if (FAILED(result)) {
    printf("Error @ EnumOutputs\n");
    goto RELEASE;
  }

  // DXGIOutputの情報を取得
  DXGI_OUTPUT_DESC dxgi_output_descriptor;
  dxgi_output->GetDesc(&dxgi_output_descriptor);

  // Descriptorの中身を表示
  printf("=== DXGI_OUTPUT_DESC ===\n");
  _tprintf(TEXT("Device Name: %s\n"), dxgi_output_descriptor.DeviceName);
  if (dxgi_output_descriptor.AttachedToDesktop) printf("Attached to Desktop: true\n");
  printf("Desktop Coordinates: (%d,%d) to (%d,%d)\n",
      dxgi_output_descriptor.DesktopCoordinates.left,
      dxgi_output_descriptor.DesktopCoordinates.top,
      dxgi_output_descriptor.DesktopCoordinates.right,
      dxgi_output_descriptor.DesktopCoordinates.bottom);

  // DXGIOutput1 = Desktop Duplication可能な出力先(モニタ)に変換
  IDXGIOutput1 *dxgi_output_one = nullptr;
  result = dxgi_output->QueryInterface(__uuidof(dxgi_output_one),
      reinterpret_cast<void**>(&dxgi_output_one));
  dxgi_output->Release();
  dxgi_output = nullptr;
  if (FAILED(result)) {
    printf("Error @ Query DXGIOutput1\n");
    goto RELEASE;
  }

  // やっと目的のOutput Duplicationを取得
  IDXGIOutputDuplication *dxgi_output_duplication = nullptr;
  result = dxgi_output_one->DuplicateOutput(device, &dxgi_output_duplication);
  dxgi_output_one->Release();
  dxgi_output_one = nullptr;
  if (FAILED(result)) {
    if (result == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
      printf("Error @ Too many duplications\n");
    }
    printf("Error @ Duplicate Output\n");
    goto RELEASE;
  }

  // Output Duplicationの情報を表示
  DXGI_OUTDUPL_DESC dxgi_output_duplication_descriptor;
  dxgi_output_duplication->GetDesc(&dxgi_output_duplication_descriptor);
  printf("=== DXGI_OUTDUPL_DESC ===\n");
  if (dxgi_output_duplication_descriptor.DesktopImageInSystemMemory) {
    printf("DesktopImageInSystemMemory: true\n");
  } 
  printf("Size/FPS: (%d, %d) %dfps\n",
      dxgi_output_duplication_descriptor.ModeDesc.Width,
      dxgi_output_duplication_descriptor.ModeDesc.Height,
      (int)(dxgi_output_duplication_descriptor.ModeDesc.RefreshRate.Numerator /
            dxgi_output_duplication_descriptor.ModeDesc.RefreshRate.Denominator));

#ifdef MAP_DESKTOP_SURFACE
  // MapDesktopSurfaceのテスト
  // だめそうなのでコメントアウト
  if (dxgi_output_duplication_descriptor.DesktopImageInSystemMemory) {
    const int width = dxgi_output_descriptor.DesktopCoordinates.right -
        dxgi_output_descriptor.DesktopCoordinates.left;
    const int height = dxgi_output_descriptor.DesktopCoordinates.bottom -
        dxgi_output_descriptor.DesktopCoordinates.top;
    const int data_size = width * height * 4;
    auto buffer = new BYTE[data_size];

    DXGI_MAPPED_RECT desktop_surface;
    result = dxgi_output_duplication->MapDesktopSurface(&desktop_surface);
    if (FAILED(result)) {
      printf("Error @ MapDesktopSurface %d\n", result);
      goto RELEASE;
    }

    memcpy(buffer, desktop_surface.pBits, data_size);
    
    result = dxgi_output_duplication->UnMapDesktopSurface();
    if (FAILED(result)) {
      printf("Error @ UnMapDesktopSurface\n");
      goto RELEASE;
    }

    SaveBitmapFile(buffer, width, height, TEXT("test2.bmp"));

    delete [] buffer;
  }
#endif

  // 一回だけ適当にAcquireNextFrame
  DXGI_OUTDUPL_FRAME_INFO frame_info;
  IDXGIResource *dxgi_resource = nullptr;
  result = dxgi_output_duplication->AcquireNextFrame(0, &frame_info, &dxgi_resource);
  if (FAILED(result)) {
    if (result == DXGI_ERROR_ACCESS_LOST) {
      printf("Error DXGI_ERROR_ACCESS_LOST @ First AcquireNextFrame\n");
    } else if (result == DXGI_ERROR_WAIT_TIMEOUT) {
      printf("Error DXGI_ERROR_WAIT_TIMEOUT @ First AcquireNextFrame\n");
    } else {
      printf("Error @ First AcquireNextFrame\n");
    }
    goto RELEASE;
  }
  if (dxgi_resource != nullptr) {
    dxgi_resource->Release();
    dxgi_resource = nullptr;
  }
  dxgi_output_duplication->ReleaseFrame();

  // Timeout付きで取得
  const int timeout_milliseconds = INFINITE;
  result = dxgi_output_duplication->AcquireNextFrame(timeout_milliseconds, &frame_info, &dxgi_resource);
  if (FAILED(result)) {
    if (result == DXGI_ERROR_WAIT_TIMEOUT) {
      printf("Error @ Timeout\n");
    }
    printf("Error @ AcquireNextFrame\n");
    goto RELEASE;
  }

  // このままでは得られたResourceがTextureなのかBufferなのかわからないのでTextureにする
  ID3D11Texture2D *d3d11_texture = nullptr;
  result = dxgi_resource->QueryInterface(__uuidof(ID3D11Texture2D),
      reinterpret_cast<void**>(&d3d11_texture));
  dxgi_resource->Release();
  dxgi_resource = nullptr;
  if (FAILED(result)) {
    printf("Error @ Query ID3D11Texture2D\n");
    goto RELEASE;
  }

  // D3D11_TEXTURE2D_DESCの取得
  D3D11_TEXTURE2D_DESC d3d11_texture_descriptor;
  d3d11_texture->GetDesc(&d3d11_texture_descriptor);

  // Descriptorの中身を表示
  printf("=== D3D11_TEXTURE2D_DESC ===\n");
  printf("Captured Width: %d\n", d3d11_texture_descriptor.Width);
  printf("Captured Height: %d\n", d3d11_texture_descriptor.Height);

  // 試しに少し小さなテクスチャにコピーしてみる
  D3D11_BOX source_region;
  source_region.left = 0;
  source_region.right = 320;
  source_region.top = 0;
  source_region.bottom = 240;
  source_region.front = 0;
  source_region.back = 1;

  // システムメモリ上にStaging Bufferを作成する準備
  D3D11_TEXTURE2D_DESC system_memory_texture_descriptor;
  RtlZeroMemory(&system_memory_texture_descriptor, sizeof(system_memory_texture_descriptor));
  system_memory_texture_descriptor.Width = source_region.right - source_region.left;
  system_memory_texture_descriptor.Height = source_region.bottom - source_region.top;
  system_memory_texture_descriptor.MipLevels = 1;
  system_memory_texture_descriptor.ArraySize = 1;
  system_memory_texture_descriptor.Format = d3d11_texture_descriptor.Format;
  system_memory_texture_descriptor.SampleDesc.Count = 1;
  system_memory_texture_descriptor.Usage = D3D11_USAGE_STAGING;
  system_memory_texture_descriptor.BindFlags = 0;
  system_memory_texture_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  system_memory_texture_descriptor.MiscFlags = 0;
  
  // Staging Bufferを作成
  ID3D11Texture2D *system_memory_texture = nullptr;
  result = device->CreateTexture2D(&system_memory_texture_descriptor,
      nullptr, &system_memory_texture);
  if (FAILED(result)) {
    printf("Error @ CreateTexture2D\n");
    goto RELEASE;
  }

  // StagingBufferにコピー
  /// @todo(me): CopySubresourceRegionで与えられたRECTの部分のみを転送
  // おそらくCopy前にsystem_memory_textureを黒で塗りつぶして置かなければならないはず
  // クリッピングはしてくれないのでこの前にIntersectRectするしかない
  const int destination_left = 0;
  const int destination_top = 0;
  device_context->CopySubresourceRegion(system_memory_texture, 0,
      destination_left, destination_top, 0,
      d3d11_texture, 0, &source_region);
  // @todo(me): 要調査。どうも自分のCopyResourceへの理解が間違っている気がする
  // ここに移動しても動作しない場合がある
  d3d11_texture->Release();
  d3d11_texture = nullptr;

  // d3d11_textureがいらなくなった段階でReleaseFrameする
  result = dxgi_output_duplication->ReleaseFrame();
  if (FAILED(result)) {
    printf("Error @ ReleaseFrame\n");
    goto RELEASE;
  }

  // 直接MapしてもいいがSurfaceを解するとプログラムが見やすくなる
  IDXGISurface *system_memory_surface = nullptr;
  result = system_memory_texture->QueryInterface(__uuidof(IDXGISurface),
      reinterpret_cast<void**>(&system_memory_surface));
  system_memory_texture->Release();
  system_memory_texture = nullptr;
  if (FAILED(result)) {
    printf("Error @ Query IDXGISurface\n");
    goto RELEASE;
  }

  // Map
  DXGI_MAPPED_RECT mapped_surface;
  result = system_memory_surface->Map(&mapped_surface, DXGI_MAP_READ);
  if (FAILED(result)) {
    printf("Error @ Map\n");
    goto RELEASE;
  }

  // あとはmemcpyすればavpicture化できる
  if (mapped_surface.pBits != nullptr) {
    // 試しにBitmap出力してみる
    SaveBitmapFile(mapped_surface.pBits,
        system_memory_texture_descriptor.Width,
        system_memory_texture_descriptor.Height,
        TEXT("test.bmp"));

    printf("=== BMP FILE SAVED ===\n");
  } else {
    printf("Something is wrong\n");
  }

  // 使い終わったらUnmapすればOK
  result = system_memory_surface->Unmap();
  if (FAILED(result)) {
    printf("Error @ Unmap\n");
    goto RELEASE;
  }

RELEASE:
  // 開放
  if (system_memory_surface != nullptr) {
    system_memory_surface->Release();
    system_memory_surface = nullptr; 
  }
  if (d3d11_texture != nullptr) {
    d3d11_texture->Release();
    d3d11_texture = nullptr;
  }
  if (dxgi_output_duplication != nullptr) {
    dxgi_output_duplication->Release();
    dxgi_output_duplication = nullptr;
  }
  if (device_context != nullptr) {
    device_context->Release();
    device_context = nullptr;
  }
  if (device != nullptr) {
    device->Release();
    device = nullptr;
  }
}

int _tmain(int argc, _TCHAR* argv[]) {
  //TestFFDraw();
  printf("scff_sandbox\n");
  TestDXGIDesktopDuplication();
  getchar();
  return 0;
}
