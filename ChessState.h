#ifndef CHESSSTATE_H
#define CHESSSTATE_H

#include <vector>
#include <ChessPieces.h>

#define BLACK false
#define RED true

class ChessState
{
public:
    bool currentTurn = BLACK;

    ChessState() = default;
    ChessState(std::vector<ChessPiece> pieces,bool currentTurn = BLACK);
    ChessState(ChessPiece pieces[],bool currentTurn = BLACK);
    std::vector<ChessPiece> getChessPieces(){
        return this->chessPieces;
    }

    bool playoutUntilEnd();
    std::vector<ChessState> getAllPossibleNextState();

private:
    //all chess pieces, contains position infos
    std::vector<ChessPiece> chessPieces;
    ChessState suggestedNextState();

};

#endif // CHESSSTATE_H
