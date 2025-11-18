#include "Chess.h"
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
    for(int i=0; i<64; i++) {
        _knightBitboards[i] = generateKnightMoveBitboard(i);
    }
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
}

// capitals are white and lower are black pieces 
void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });

    std::string placement = fen.substr(0, fen.find(' '));

    int row = 0;
    int col = 0;

    // move through character in the fen string
    for (char spot : placement) {
        // after a / meanns we are in a new row
        if (spot == '/') {
            row++;
            col = 0;
            continue;
        }
        // numbers mean empty spaces so skip
        if (isdigit(spot)) {
            col += spot - '0'; // ascii trick to keep track of column
            continue;
        }

        int playerNumber = isupper(spot) ? 0 : 1; // white : black
        ChessPiece pieceType;

        // get the piece in accordance to letter
        switch (tolower(spot)) {
            case 'p': pieceType = Pawn; break;
            case 'n': pieceType = Knight; break;
            case 'b': pieceType = Bishop; break;
            case 'r': pieceType = Rook; break;
            case 'q': pieceType = Queen; break;
            case 'k': pieceType = King; break;
            default: continue;
        }

        int gridY = 7 - row; 
        int gridX = col;
        
        // get the piece example: 0, k = white king
        Bit* piece = PieceForPlayer(playerNumber, pieceType);
        
        auto square = _grid->getSquare(gridX, gridY);
        // check if square exists and place piece on square, set square as parent and move the piece to it 
        if (square) {
            square->setBit(piece);
            piece->setParent(square);
            piece->moveTo(square->getPosition());
        }

        col++;
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return true;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t emptySquares) {
    knightBoard.forEachBit([&](int fromSquare) {
        BitBoard moveBitboard = BitBoard(_knightBitboards[fromSquare].getData() & emptySquares);
        moveBitboard.forEachBit([&](int toSquare) {
           moves.emplace_back(fromSquare, toSquare, Knight);
        });
    });
}

BitBoard Chess::generateKnightMoveBitboard(int square) {
    BitBoard bitboard = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    std::pair<int, int> knightOffsets[] = {
        { 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 },
        { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }
    };

    constexpr uint64_t oneBit = 1;
    for (auto [dr, df] : knightOffsets) {
        int r = rank + dr, f = file + df;
        if (r >= 0 && r < 8 && f >= 0 && f < 8) {
            bitboard |= oneBit << (r * 8 + f);
        }
    }
    
    return bitboard;
}
