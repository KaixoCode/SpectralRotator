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

Any samples you've dragged out of this plugin are stored in the output folder, which defaults to a folder somewhere in the user's application data directory. You can find the exact path by clicking on the settings icon and looking at the "output folder". You can change this path by clicking on it and selecting a different folder. Clicking on the icon to the right of the path opens the folder in the default file explorer. You can close the settings by clicking on the settings icon again.

![settings](https://assets.kaixo.me/SpectralRotator/settings-ui.png)

The spectrum view is also fully customizable, you can adjust the FFT size, resolution, block size, and range. The FFT size changes the vertical resolution, the FFT resolution changes the horizontal resolution, the block size determines the length of the block that is used for every FFT, and the FFT range adjusts how many decibels are visible.

**An important note** when dropping modified rotated samples back into SpectralRotator is that you align the bottom of your spectrum properly. For example, when you have rotated your spectrum such that the lower frequencies are on the left, you must make sure when you import your edited sample, that the start of your sample is aligned (If it was rotated such that the lower frequencies are on the right, you would have to align the end of your sample). Otherwise a small frequency shift will occur, depending on how many audio samples of delay there are. You do not have to worry about making sure the sample is exactly the same length though, as it will take this information from the *source* sampler. 

## Questions
If you have any questions or suggestions about this plugin you can contact me on Discord `@kaixo`.