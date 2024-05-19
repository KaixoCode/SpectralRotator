# SpectralRotator
SpectralRotator is a VST plugin that can rotate the frequency spectrum of any sound. It is an offline processor, which means it does not work on live audio. Instead, you drop a sample into its UI, rotate its frequency spectrum in any direction, and then drag the result back into the DAW.



## Installation Guide
Installing this VST is very straightforward. Go to the [Releases](https://github.com/KaixoCode/SpectralRotator/releases) tab, select the latest release, and simply download the VST3 file. You then put this VST3 file into your VST3 folder, it is usually located somewhere like `C:\Program Files\Common Files\VST3`.

Unfortunately I do not have access to an iOS device to compile it for Mac, so **there is currently only a Windows version available**.

## How To Use The Plugin
When you open the plugin you are faced with two simple samplers: *source* and *rotate*. Start by dropping a sample into the *source* sampler, this will load the sample in both *source* and *rotate*. You can then press the buttons on the *rotate* sampler to rotate the frequency spectrum into any orientation. Once you've rotated the sample, you can drag the result back into your DAW by hovering over the "rotate" text and dragging.

![rotated sample](https://assets.kaixo.me/SpectralRotator/rotated-sample-ui.png)

After you've applied effects to your rotated sample, you can drop it back into the *rotate* sampler. Make sure you still have the original source sample in the *source* sampler. This is important because we need to know how long the original sample was to rotate it back properly, as the length of your edited sample might have changed. Once the edited sample has loaded, you can rotate it back to its original orientation, and drag it back into your DAW.

You can also listen to the loaded samples by clicking on the spectrum view, and pressing space bar. This will play/pause playback of this sampler. You can skip to any position in the sample by clicking anywhere on the spectrum view.

Any samples you've dragged out of this plugin are stored in a folder somewhere in the user's application data directory. You can find the exact path by clicking on the settings icon and looking at the "output folder", you can also click on this to open the folder in the default file explorer. You can close the settings by clicking on the settings icon again.

![settings](https://assets.kaixo.me/SpectralRotator/settings-ui.png)

## Questions
If you have any questions or suggestions about this plugin you can contact me on Discord `@kaixo`.