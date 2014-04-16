// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkId=232509


var selectedFile;

function LoadImage() {
    ClearEditedImage(); //In case another image was loaded before
    var picker = new Windows.Storage.Pickers.FileOpenPicker();
    picker.viewMode = Windows.Storage.Pickers.PickerViewMode.thumbnail;
    picker.suggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.picturesLibrary;
    picker.fileTypeFilter.replaceAll([".png", ".jpg", ".jpeg"]);

    picker.pickSingleFileAsync().then(
        function (selected) {
            selectedFile = selected;
            var url = URL.createObjectURL(selected, { oneTimeOnly: true });
            var imgTag = document.getElementById("origImage");
            imgTag.src = url;
        });
}

function UpdateProgress(percent) {
    var progress = document.getElementById('progress');
    progress.style.width = (0.9 * percent) + "%";
    progress.style.visibility = "visible";

    var dbg = document.getElementById('dbg');
    dbg.textContent = percent + "%";
}

function ClearEditedImage() {
    var eimgTag = document.getElementById("editedImage");
    eimgTag.src = "";
}

function TransformImage(isParallel) {
    ClearEditedImage();
    UpdateProgress(0);
    var transformer = new ImageCartoonizerServer.ImageTransformer(isParallel);
    //
    // Copy the file to a temporary folder to avoid access violation issues
    //
    selectedFile.copyAsync(Windows.Storage.ApplicationData.current.temporaryFolder, selectedFile.fileName, Windows.Storage.NameCollisionOption.replaceExisting).then(

        function (copiedFile) {
            var newFileName = "edited_" + selectedFile.fileName;
            var dstImgPath = Windows.Storage.ApplicationData.current.temporaryFolder.path + "\\" + newFileName;
            transformer.getTransformImageAsync(copiedFile.path, dstImgPath).then(
                function () {
                    UpdateProgress(100);
                    Windows.Storage.ApplicationData.current.temporaryFolder.getFileAsync(newFileName).then(
                        function (fileObject) {
                            var newurl = URL.createObjectURL(fileObject, { oneTimeOnly: true });
                            var eimgTag = document.getElementById("editedImage");
                            eimgTag.src = newurl;
                        });
                },
                function (error) {
                    // An error has occurred
                    UpdateProgress(0);
                },
                function (progressPercent) {
                    UpdateProgress(progressPercent);
                }
            );
        });
}

(function () {
    "use strict";

    var app = WinJS.Application;
    var activation = Windows.ApplicationModel.Activation;

    app.onactivated = function (args) {
        if (args.detail.kind === activation.ActivationKind.launch) {
            args.setPromise(WinJS.UI.processAll());
        }
    };

    app.start();
})();
