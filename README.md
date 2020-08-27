# Audio Manager

This project is a Blackjack Game (no graphic display) that implements an Audio Engine built in from scratch in C++ using FMOD low-level API.

The Audio Manager class handles loading, playing and stopping songs and effects, as well as managing the volume of the channels, fades in and out, generating random volume and pitch values within a range (to make sfxs less repetitive), setting up the listener properties and setting up a 3D reverb environement.



I am currently working in a new Audio Engine written in C++ (FMOD low-level API) with the following features:
* All load, play, randomize, stop, fade in/out, reverb zones and dsp effects on channel groups basic functionalities &
* A machine state that handles the AudioManager.Update() method. It will perform all the calls to the middleware API (so from game code only setting flags is needed, e.g. bIsStopped, bIsVirtual, bIsStopping...).
    
In the future I plan to update this whole repository with an AAA-style Unreal Engine demo that uses my Audio Engine.
