// Cartoonizer.cpp
#include "pch.h"
#include "Cartoonizer.h"

using namespace ImageCartoonizerServer;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Foundation; 
using namespace Windows::Graphics::Display; 
using namespace Windows::Storage; 
using namespace Windows::Storage::Pickers; 
using namespace Windows::Storage::Streams; 

ImageTransformer::ImageTransformer(bool isParallel)
{
	isParallelTransform = isParallel;
}

IAsyncActionWithProgress<int>^ ImageTransformer::GetTransformImageAsync(String^ inFile, String^ outFile)
{
    return create_async([=](progress_reporter<int> progress) {
        TransformImage(inFile, outFile, progress);
    });
}

void ImageTransformer::TransformImage(String^ inFile, String^ outFile, progress_reporter<int> progress)
{
    HRESULT hr;

    hr = LoadFromImageFile(inFile);
    if(SUCCEEDED(hr))
        hr = InternalTransformImage(progress);
    if(SUCCEEDED(hr))
        hr = SaveToImageFile(outFile);
}

HRESULT ImageTransformer::GetImageSize()
{
    HRESULT hr = wicBitmap->GetSize(&width, &height);
    return hr;
}

HRESULT ImageTransformer::LoadFromImageFile(String^ wFileName)
{
    HRESULT hr = S_OK;

    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICBitmapFrameDecode> bitmapSource;

    hr = CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,&wicFactory);

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateDecoderFromFilename(wFileName->Data(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&decoder);

    if (SUCCEEDED(hr))
        hr = decoder->GetFrame(0, &bitmapSource);

    if(SUCCEEDED(hr))
        hr = bitmapSource.As<IWICBitmapSource>(&wicBitmap);

    if(SUCCEEDED(hr))
        GetImageSize();

    return hr;
}

HRESULT ImageTransformer::InternalTransformImage(progress_reporter<int> progress)
{
    HRESULT hr = S_OK;
    WICBitmapTransformOptions options = WICBitmapTransformFlipHorizontal ;

    hr = wicFactory->CreateBitmapFlipRotator(&FlipRotator);
    if(SUCCEEDED(hr))
        hr = FlipRotator->Initialize(wicBitmap.Get(),options);

    if (SUCCEEDED(hr))
    {
        wicBitmap.Attach(FlipRotator.Detach());
        hr = GetImageSize();
    }

    ComPtr<IWICBitmap> bitmap = NULL;

    WICRect rcLock = { 0, 0, width, height };
    IWICBitmapLock *pLock = NULL;

    hr = wicFactory->CreateBitmapFromSource(wicBitmap.Get(), WICBitmapCacheOnDemand, &bitmap);
    if (SUCCEEDED(hr))
    {
        hr = bitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);

        if (SUCCEEDED(hr))
        {
            BYTE* pFrame = NULL;
            UINT cbBufferSize = 0;
            UINT cbStride = 0;

            hr = pLock->GetStride(&cbStride);

            if (SUCCEEDED(hr))
            {
                hr = pLock->GetDataPointer(&cbBufferSize, &pFrame);
            }

            WICPixelFormatGUID format;
            wicBitmap->GetPixelFormat(&format);

          	int bpp;
			if (format == GUID_WICPixelFormat8bppGray || format == GUID_WICPixelFormat8bppAlpha || format == GUID_WICPixelFormat8bppIndexed)
			{
				bpp = 8;
			}
			else if (format == GUID_WICPixelFormat24bppRGB || format == GUID_WICPixelFormat24bppBGR)
			{
				bpp = 24;
			}
			else throw;
			
			
            unsigned int size = cbBufferSize;

            const int neighbourWindow = 3;
            const int phasesCount = 3;

            FrameData frameData;
            frameData.m_BBP = bpp;
            frameData.m_ColorPlanes = 1;
            frameData.m_EndHeight = height;
            frameData.m_EndWidth = width;
            frameData.m_neighbourArea = neighbourWindow;
            frameData.m_pFrame = pFrame;
            frameData.m_pFrameProcesser = NULL;
            frameData.m_PhaseCount = phasesCount;
            frameData.m_Pitch = cbStride;
            frameData.m_Size = size;
            frameData.m_StartHeight = 0;
            frameData.m_StartWidth = 0;

            FrameProcessing frameProcessing;
            frameData.m_pFrameProcesser = &frameProcessing;

            frameData.m_pFrameProcesser->SetNeighbourArea(frameData.m_neighbourArea);
            frameData.m_pFrameProcesser->SetCurrentFrame(frameData.m_pFrame, frameData.m_Size, frameData.m_EndWidth,
                                                            frameData.m_EndHeight, frameData.m_Pitch, frameData.m_BBP, frameData.m_ColorPlanes);

            frameData.m_pFrameProcesser->ApplyFilters(frameData.m_PhaseCount, isParallelTransform, [progress](int percent) { progress.report(percent); });
            frameData.m_pFrameProcesser->FrameDone(pFrame, size);

            // Release the bitmap lock.
            pLock->Release();
            wicBitmap = bitmap;
        }
    }
    return hr;
}

HRESULT ImageTransformer::SaveToImageFile(String^ outFile)
{
    HRESULT hr = S_OK;

    FILE* fHandle;
    if (_wfopen_s(&fHandle, outFile->Data(), L"wb"))
        throw ref new Platform::AccessDeniedException();

    fclose(fHandle);

    ComPtr<IWICBitmapEncoder> pEncoder = NULL;
    ComPtr<IWICBitmapFrameEncode> pFrameEncode = NULL;
    ComPtr<IWICStream> pStream = NULL;

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateStream(&pStream);


    WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
    if (SUCCEEDED(hr))
        hr = pStream->InitializeFromFilename(outFile->Data(), GENERIC_WRITE);

    if (SUCCEEDED(hr))
        hr = wicFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);

    if (SUCCEEDED(hr))
        hr = pEncoder->Initialize(pStream.Get(), WICBitmapEncoderNoCache);

    if (SUCCEEDED(hr))
        hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);

    if (SUCCEEDED(hr))
        hr = pFrameEncode->Initialize(NULL);

    if (SUCCEEDED(hr))
        hr = pFrameEncode->SetSize(width, height);

    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->WriteSource(wicBitmap.Get(), NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = pFrameEncode->Commit();
    }
    if (SUCCEEDED(hr))
    {
        hr = pEncoder->Commit();
    }

    return hr;
}
