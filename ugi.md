# Universal Game Interface
The UGI protocol is based on the UCI protocol used in chess. The only meaningful change besides generalisation is the addition of the ```query``` and the ```response``` commands.

---

## ugi
```ugi``` must be sent before any other command and replied to with ```ugiok```

---

## isready
```isready``` can be sent at any time and must be replied to with ```readyok``` whenever the engine is ready. ```isready``` will be sent once after ```ugi``` before the engine is asked to search/etc. No large amounts of memory or other time expensive operations should be performed before the first ```isready``` is received.

---

## Query
Various queries can be made to the engine about the current game state. Queries must be sent in the form ```query [question]``` and must be responded to with ```response [answer]```
```
gameover -- has the game ended (true/false)
p1turn   -- is it currently the first player's turn (true/false)
result   -- result of the position (p1win/p2win/draw/none)
```

---

## go
The ```go``` command varies depending on the time control being used.
```
go p1time [ms] p2time [ms] p1inc [ms] p2inc [ms]
go movetime [ms]
go depth [ply]
go nodes [nodes]
go infinite
```
The engine indicates the search has finished with
```
bestmove [move string]
```

---

## stop
Stops any ongoing search.

---

## position
```
position fen [fen]
position startpos
position fen [fen] moves [moves list]
position startpos moves [moves list]
```

---

## Example Usage

```
<send> ugi
<recv> id name EngineName
<recv> id author EngineAuthor
<recv> option name hash type spin default 128 min 1 max 2048
<recv> ugiok
<send> setoption name hash value 512
<send> isready
<recv> readyok
<send> uginewgame
<send> position startpos
<send> query p1turn
<recv> response true
<send> go depth 4
<recv> info depth 1 seldepth 1 score cp -107 time 0 nodes 16 tthits 0 hashfull 0 pv f1
<recv> info depth 2 seldepth 2 score cp 200 time 0 nodes 50 tthits 1 hashfull 0 pv f1 b1
<recv> info depth 3 seldepth 3 score cp -57 time 0 nodes 197 tthits 12 hashfull 0 pv f1 b1 g2
<recv> info depth 4 seldepth 4 score cp 200 time 0 nodes 473 tthits 38 hashfull 0 pv f1 b1 g2 a2
<recv> bestmove f1
<send> position startpos moves f1
<send> query p1turn
<recv> response false
<send> query result
<recv> response none
<send> quit
```
