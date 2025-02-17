#include "ChessRuleProvider.h"
#include "ChessStep.h"
#include "Rule.h"


using namespace rule;

std::vector<ChessState> ChessRuleProvider::getAllPossibleChildState(ChessState* state){
    std::vector<ChessStep> stepsToMoveForAllPieces = getAllValidSteps(state);
    // generate state for all possible steps
    std::vector<ChessState> resStates;
    for(auto const &step:stepsToMoveForAllPieces){
        auto const &resState = getState(step,state);
        resStates.push_back(resState);
    }
    return resStates;
}

std::vector<ChessStep> ChessRuleProvider::getAllValidSteps(ChessState* state){
    std::vector<ChessStep> stepsToMoveForAllPieces;
    // for all the pieces in current state that is moveable, generate it's possible steps
    for(auto const &piece:state->getChessPieces()){
        if(piece.isRed == state->currentTurn && !piece.isDead){ // only move for the current turn
            auto const &stepsForOnePiece = getValidStepsForPiece(piece,state);
            stepsToMoveForAllPieces.insert(stepsToMoveForAllPieces.end(),stepsForOnePiece.begin(),stepsForOnePiece.end());
        }
    }

    //handle the kill steps, the piece on destination must be enemy, because it is valid
    for(auto &step:stepsToMoveForAllPieces){
        ChessPiece* pieceOnDestination = state->getPieceByPos(step.toRow,step.toCol);
        if(pieceOnDestination!=nullptr){
            step.killId = pieceOnDestination->id;
        }
    }

    return stepsToMoveForAllPieces;
}


bool ChessRuleProvider::isGameEnd(ChessState* state){
    for(auto &p:state->getChessPieces()){
        if(p.type == ChessPiece::JIANG && p.isDead){
            return true;
        }
    }
    return false;
}

bool ChessRuleProvider:: whoWins(ChessState* state){
    for(auto &p:state->getChessPieces()){
        if(p.type == ChessPiece::JIANG && p.isDead){
            return !p.isRed;
        }
    }
    throw "nobody wins";
}

// ----------------------------------------------------------------------------------- private
std::vector<ChessStep> ChessRuleProvider::getValidStepsForPiece(ChessPiece const &piece,ChessState *state){
    // generate possible steps without dependency of the current state;
    std::vector<ChessStep> stpes = rule::generateUnblockingStepsForOnePiece(piece);

    // filter out invalid step
    std::vector<ChessStep> result;
    for(auto &step:stpes){
        if(rule::validStep(step,state)){
            result.push_back(step);
        }
    }
    return result;
}

// get one state from one step
ChessState ChessRuleProvider::getState(ChessStep const &step,ChessState *state){
    ChessState res(state->getChessPieces(),!state->currentTurn);
    res.move(step);
    return res;
}
