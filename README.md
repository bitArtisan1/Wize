# Wize
Wize UCI Chess Engine with Multithreading and NNUE Stockfish Evaluation
Wize, a C++ UCI-compliant chess engine, integrates a neural network (NNUE) for accurate evaluation and employs a range of advanced search techniques like LazySMP and Multithreading within Alpha-Beta framework, Wize strives to be a top-tier engine but also serves as a valuable resource for developers to enhance their own chess engines.

![Wize, Strong and Open Source C++ Chess Engine]()
                         
<p align="center">
<em><strong>Wize, Strong and Open Source Chess Engine Written in C++</strong></em>
</p>

## Features
* Architecture
  * Bitboard Representation

* Search 
  * Alpha-Beta Pruning
  * Negamax Framework
  * Transposition Tables
  * Principal Variation Search
  * Quiescence Search
  * Null-move Pruning
  * Internal Iterative Deepening
  * Aspiration Window
  * Repetition Detection
  * Killer Move, History
  * MVV-LVA Capture Ordering
  * LazySMP
  * Multithreading
  * Heuristic Moves
  * Polyglot Opening Book
  
* Evaluation
  * NNUE Evaluation (Features=HalfKP(Friend)[41024->256x2],Network=AffineTransform[1<-32](ClippedReLU[32](AffineTransform[32<-32](ClippedReLU[32](AffineTransform[32<-512](InputSlice[512(0:512)]))))))

## Improvements
  * Implement Syzygy Bases, SyzygyPath, ...
  * Improve the NN Structure
## Current ELO
  * **ELO: 3000~** 

 ## Requirements
 * Wize software requires GCC(GNU Compiler Collection) or CLang, C++ version: CXX >= C++17 !
 * It is recommended that the terminal uses a TrueType Font (ttf), Consolas and other monospaced fonts are recommended for the best user experience [Unicode characters and ANSI escape code] (However, most engines are 
   linked directly to the GUI and don't really need to interact with the terminal).
 
 ## Installation/Usage
 Clone the repository:

```bash
$ git clone 
```
Then, in the directory:
```bash
$ cd Wize
$ make
```
Run the generated .exe file in the install dir.
 
* How to work the engine?
The UCI (Universal Chess Interface) serves as a widely adopted protocol for interacting with a chess engine and is the preferred method for communication in graphical user interfaces (GUIs) and chess-related tools. The guidelines are outlined in the UCI protocol, which can be accessed and referenced at this link http://page.mi.fu-berlin.de/block/uci.htm


## Acknowledgements
* [BluefeverSoftware](https://www.youtube.com/@BlueFeverSoft), YouTube series about Vice engine.
* [CodeMonkeyKing](https://www.youtube.com/@chessprogramming591), General chess programming videos.
* Vice Programming Discord
* And of course, [ChessProgrammingWik](https://www.chessprogramming.org/Main_Page).

## Images
### Terminal interface
![Terminal interface]()
)

### Gameboard
![Gameboard]()
)




