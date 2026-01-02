# MilkDropX Music Visualizer

This is a stand-alone implementation of the amazing [MilkDrop2 music visualizer](https://www.geisswerks.com/about_milkdrop.html).

It is available as a linkable DLL for applications and as a stand-alone EXE player thanks to the work of the [BeatDrop team](https://github.com/mvsoft74/BeatDrop). It includes an x64 port using [projectm-eval](https://github.com/projectM-visualizer/projectm-eval) and maintains support for x86 with the original ns-eel2 library. It has also been ported to build with Visual Studio 2022 and newer.

Contributions welcome!

## System Requirements
* Windows 10, Windows 8.1 or Windows 7 SP1
* WASAPI-compatible sound card (for stand-alone)
* DirectX 9 compatible GPU (or you can use [DXVK](https://github.com/doitsujin/dxvk) for Vulkan)
* DirectX End-User [Runtimes](https://www.microsoft.com/en-us/download/details.aspx?id=8109) (also included in the installer) contains the required 32-bit helper libraries d3dx9_43.dll and d3dx9_31.dll

## Acknowledgements
Special thanks to:
* Kai Blaschke and the projectM team [projectM Expression Evaluation Library](https://github.com/projectM-visualizer/projectm-eval)
* Maxim Volskiy [BeatDrop](https://github.com/mvsoft74/BeatDrop)
* Ryan Geiss and Rovastar (John Baker) [official Milkdrop2 source code](https://sourceforge.net/projects/milkdrop2/)
* oO-MrC-Oo [XBMC plugin](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde [loopback-capture](https://github.com/mvaneerde/blog)
* and all the preset authors!

## License

This repository is licensed under the [3-Clause BSD License](LICENSE).

The projectM Expression Evaluation Library ("projectm-eval") is licensed under the [MIT license](https://github.com/projectM-visualizer/projectm-eval/blob/master/LICENSE.md).

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide the [license clarification](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/)

> ### Sunny March 29, 2015 at 11:06 pm
> Hi. Just wondering is this open source? I'm looking for something like this for my school project.
>
> ### Maurits [MSFT] March 30, 2015 at 8:35 am
> @Sunny do with the source as you like.

All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.
