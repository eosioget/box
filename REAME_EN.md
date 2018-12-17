# BOX

## A sentence introduction
D&D Style BlockChain Treasure Game. Five Warriors Fighting for EOS

## Introduction to Game
BOX is the second product launched by EOSGET platform, which is a D&D style red envelope Solitaire game. The game uses the data on the block chain as the basis of prize opening. The amount of money that each player grabs is completely fair, random and verifiable.

## Algorithm
### Brief description
The BOX prize-opening pipeline is calculated on the basis of the next `block_id'of the time when the treasure box was finally seized.

### An example of a case
The last time to grab the treasure box (5 persons) is `time', and the next `block_id'of `time > time', assuming that `block_id' is:
![](block_id.png)

Take the last 10 digits and take out 5 digits `25 45 98 86 58` for each two digits in a group.

The sum of five groups of numbers is `25 + 45 + 98 + 86 + 56 = 310`.

The first player will get: `25/310 * 1 EOS = 0.0806 EOS`.  
The second player will get: `45/310 * 1 EOS = 0.1451 EOS`.  
The third player will get `98/310 * 1 EOS = 0.3161 EOS`.  
The fourth player will get: `86/310 * 1 EOS = 0.2774 EOS`.  
The fifth player will get: `56/310 * 1 EOS = 0.1806 EOS`.  