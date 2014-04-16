#pragma once

using namespace concurrency;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;

namespace ImageCartoonizerServer
{
	public ref class ImageTransformer sealed
    {
    public:

        //
        // Expose image transformation as an asynchronous action with progress
        //
        IAsyncActionWithProgress<int>^ GetTransformImageAsync(String^ inFile, String^ outFile);
		ImageTransformer(bool isParallel);

    private:
        void TransformImage(String^ inFile, String^ outFile, progress_reporter<int> progress);

        unsigned int width, height;
        ComPtr<IWICBitmapSource> wicBitmap;
        ComPtr<IWICImagingFactory> wicFactory;
        ComPtr<IWICBitmapScaler> scaler;
        ComPtr<IWICBitmapFlipRotator> FlipRotator;

        HRESULT GetImageSize();
        HRESULT LoadFromImageFile(String^ wFileName);
        HRESULT InternalTransformImage(progress_reporter<int> progress);
        HRESULT SaveToImageFile(String^ outFile);
		bool isParallelTransform;
	};
}