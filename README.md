# Cute Games
Run engine matches for a variety of games, relying on the engines themselves to validate unsupported games. Given this reliance on the engines, that means they have to be both trustworthy and well tested.

---

## __UGI Protocol__
The Universal Game Interface protocol is a text based protocol used for communication with engines. A currently incomplete overview can be found [here](ugi.md).

---

## __Why should I use this?__
Cute Games is designed to save engine developers from having to implement a program to run engine matches for every game they might be interested in. As such, it may be missing features that a specifically designed program might provide. If such a program exists for the game of interest, it could be the better option.

#### Some alternatives:
- [Cute Chess](https://github.com/cutechess/cutechess) -- Works very well for playing chess and some chess variants.
- [Cute Ataxx](https://github.com/kz04px/cuteataxx) -- For [Ataxx](https://en.wikipedia.org/wiki/Ataxx).

---

## __Limitations__
Cute Games will only work for games similar in nature to chess. Specifically, that means:
- Two player
- Alternating turns

Cute Games can handle games such as:
- chess
- checkers
- reversi
- go
- tic tac toe

Cute Games can not handle games such as:
- poker
- rock paper scissors

---

## __Build__
CMake, a C++20 compiler, and the boost library are required.
```
git clone https://github.com/kz04px/cutegames
mkdir cutegames/build
cd cutegames/build
cmake ..
cmake --build .
```

---

## __License__
Cute Games is released under the MIT license.

---

## __Thanks__
- [doctest](https://github.com/doctest/doctest) -- The fastest feature-rich C++11/14/17/20 single-header testing framework -- MIT License
- [CLI11](https://github.com/CLIUtils/CLI11) -- CLI11 is a command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface.
- [JSON](https://github.com/nlohmann/json) -- JSON for Modern C++ -- MIT LIcense
- [Termcolor](https://github.com/ikalnytskyi/termcolor) -- Termcolor is a header-only C++ library for printing colored messages to the terminal. Written just for fun with a help of the Force.
- [Boost Process](https://github.com/klemens-morgenstern/boost-process) -- Boost.Process is a library to manage system processes -- BSL 1.0
