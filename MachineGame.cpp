/*
 * Copyright (C)  2019~2020  偕臧  All rights reserved.
 *
 * Author:  xmuli(偕臧) xmulitech@gmail.com
 *
 * github:  https://github.com/xmuli
 * blogs:   https://ifmet.cn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */
#include "MachineGame.h"
#include <QApplication>
#include <chrono>
#include <thread>
#include "Node.h"
#include "MonteCarloTree.h"
#include "ChessState.h"

namespace MachineGameHelper{
    bool isSamePieceAndDifferentPos(ChessPiece a,ChessPiece b){
        if ((a.m_emType == b.m_emType) && (a.m_bRed == b.m_bRed)){
            if(a.m_nRow != b.m_nRow || a.m_nCol!=b.m_nCol){
                return true;
            }
        }
        return false;
    }
}

MachineGame::MachineGame()
{
}

MachineGame::~MachineGame()
{
}

//辅助函: 选棋或移动棋子
void MachineGame::chooseOrMovePieces(int tempID, int& row, int& col)
{
    if(m_nSelectID == -1) //选择棋子
    {
        if(m_nCheckedID != -1)
        {
            if(m_ChessPieces[m_nCheckedID].m_bRed)
            {
                m_nSelectID = tempID;
            }
            else
            {
                m_nSelectID = -1;
                return;
            }
        }
    }
    else
    {
        if(canMove(m_nSelectID, m_nCheckedID, row, col ))
        {
            //_selectId为第一次点击选中的棋子，
            //_clickId为第二次点击||被杀的棋子ID，准备选中棋子下子的地方
            m_ChessPieces[m_nSelectID].m_nRow = row;
            m_ChessPieces[m_nSelectID].m_nCol = col;
            if(m_nCheckedID != -1)
                m_ChessPieces[m_nCheckedID].m_bDead = true;

            m_nSelectID = -1;
            m_bIsRed = !m_bIsRed;
        }
    }

    whoWin();
    update();
}

void MachineGame::saveStep(int selectID, int checkedID, int row, int col, QVector<ChessStep> &steps)
{
    ChessStep step;
    step.m_nRowFrom = m_ChessPieces[selectID].m_nRow;
    step.m_nColFrom = m_ChessPieces[selectID].m_nCol;
    step.m_nRowTo = row;
    step.m_nColTo = col;
    step.m_nMoveID = selectID;
    step.m_nKillID = checkedID;

    steps.append(step);
}

void MachineGame::getAllPossibleMoveStep(QVector<ChessStep> &steps)
{
    for(int id = 0; id<16; id++)   //存在的黑棋， 能否走到这些盘棋盘里面的格子
    {
        if(m_ChessPieces[id].m_bDead)
            continue;

        for(int row=0; row<10; row++)
        {
            for(int col=0; col<9; col++)
            {
                int i = 16;
                for( ; i <= 31; i++)
                {
                    if(m_ChessPieces[i].m_nRow == row && m_ChessPieces[i].m_nCol == col && m_ChessPieces[i].m_bDead == false)
                        break;
                }

                if(i!=32)
                {
                    if(canMove(id, i, row, col))
                        saveStep(id, i, row, col, steps);
                }
            }
        }
    }
}

void MachineGame::getAllPossibleMoveStepAndNoKill(QVector<ChessStep> &steps)
{
    for(int id = 0; id<16; id++)   //存在的黑棋， 能否走到这些盘棋盘里面的格子
    {
        if(m_ChessPieces[id].m_bDead)
            continue;

        for(int row=0; row<10; row++)
        {
            for(int col=0; col<9; col++)
            {

                int i = 0;
                for(; i <= 31; i++)
                {
                    if(m_ChessPieces[i].m_nRow == row && m_ChessPieces[i].m_nCol == col && m_ChessPieces[i].m_bDead == false)
                        break;
                }

                if(id < 16 && i == 32)
                {
                    if(canMove(id, -1, row, col))
                        saveStep(id, -1, row, col, steps);
                }
            }
        }
    }
}

void MachineGame::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() != Qt::LeftButton)
        return;

    int row, col;
    if(!isSelected(ev->pos(), row, col))
        return;

    m_nCheckedID = -1;

    //TODO Fix (升级 Qt6): https://github.com/xmuli/chinessChess/issues/23
    int i =0;
    for( ; i < 32; i++)
    {
        if(m_ChessPieces[i].m_nRow == row && m_ChessPieces[i].m_nCol == col && m_ChessPieces[i].m_bDead == false)
            break;
    }

    if(0<=i && i<32)
        m_nCheckedID = i;

    clickPieces(m_nCheckedID, row, col);
}

void MachineGame::clickPieces(int checkedID, int &row, int &col)
{
    if(m_bIsRed) //红方玩家时间
    {
        chooseOrMovePieces(checkedID, row, col);
        qApp->processEvents();

        if(!m_bIsRed) //黑方紧接着进行游戏
            machineChooseAndMovePieces();
    }
}

ChessStep MachineGame::getStepFromState(ChessState state){
    ChessStep res;
    for(auto &pieceInState:state.getChessPieces()){
        for (int i = 0; i < 32; ++i) {
            auto pieceInGame = this->m_ChessPieces[i];
            bool flag = MachineGameHelper::isSamePieceAndDifferentPos(pieceInState,pieceInGame);
            if(flag){
                res.m_nColFrom = pieceInGame.m_nCol;
                res.m_nRowFrom = pieceInGame.m_nRow;

                res.m_nColTo = pieceInGame.m_nCol;
                res.m_nRowTo = pieceInGame.m_nRow;

                res.m_nMoveID = pieceInGame.m_nID;

                //TODO: killID should be assigned if kill happend
//                res.m_nKillID =
            }


        }
    }
    return res;
}



//计算最好的局面分
int MachineGame::calcScore()
{
    //enum m_emTYPE{JIANG, SHI, XIANG, MA, CHE, PAO, BING};
    //黑棋分数 - 红旗分数
    int redGrossScore = 0;
    int blackGrossScore = 0;

    static int chessScore[]={200, 20, 40, 60, 100, 80, 10};

    for(int i=0; i<16; i++)
    {
        if(m_ChessPieces[i].m_bDead)
            continue;
        blackGrossScore += chessScore[m_ChessPieces[i].m_emType];
    }

    for(int i=16; i<32; i++)
    {
        if(m_ChessPieces[i].m_bDead)
            continue;
        redGrossScore += chessScore[m_ChessPieces[i].m_emType];
    }

    return (blackGrossScore - redGrossScore);
}


//获得最好的移动步骤
ChessStep MachineGame::getBestMove()
{
    std::vector<ChessPiece> vec;
    for(auto i = 0;i<32;++i){
        vec.push_back(m_ChessPieces[i]);
    }

    MonteCarloTree<ChessState> mct(vec);

    auto bestState = mct.search();

    return this->getStepFromState(bestState);
}

void MachineGame::machineChooseAndMovePieces()
{
    ChessStep step = getBestMove();
    move(step);
}

void MachineGame::move(ChessStep step){
    if(step.m_nKillID == -1)  //黑棋没有可以击杀的红棋子，只好走能够走的过程中最后一步棋
    {
        m_ChessPieces[step.m_nMoveID].m_nRow = step.m_nRowTo;
        m_ChessPieces[step.m_nMoveID].m_nCol = step.m_nColTo;

    }
    else //黑棋有可以击杀的红棋子，故击杀红棋子
    {
        m_ChessPieces[step.m_nKillID].m_bDead = true;
        m_ChessPieces[step.m_nMoveID].m_nRow = m_ChessPieces[step.m_nKillID].m_nRow;
        m_ChessPieces[step.m_nMoveID].m_nCol = m_ChessPieces[step.m_nKillID].m_nCol;
        m_nSelectID = -1;
    }

    m_bIsRed = !m_bIsRed;
}


