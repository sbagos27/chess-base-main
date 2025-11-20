#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "MagicBitboards.h"

constexpr int pieceSize = 80;
constexpr int WHITE = +1;
constexpr int BLACK = -1;

enum AllBitBoards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
    WHITE_ALL_PIECES,
    BLACK_ALL_PIECES,
    OCCUPANCY,
    EMPTY_SQUARES,
    e_numBitboards
};

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    void clearBoardHighlights() override;

    Grid* getGrid() override { return _grid; }

private:
    char stateNotation(const char* state, int row, int col) { return state[row * 8 + col]; }
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;
    void addMoveIfValid(const char *state, std::vector<BitMove>& moves, int fromRow, int fromCol, int toRow, int toCol, ChessPiece piece);

    std::vector<BitMove> generateAllMoves();
    // knight moves
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t emptySquares);
    BitBoard generateKnightMoveBitboard(int square);

    // pawn moves
    void addPawnBitboardMovesToList(std::vector<BitMove>& moves, const BitBoard bitboard, const int shift);
    void generatePawnMoveList(std::vector<BitMove>& moves, const BitBoard pawns, const BitBoard emptySquares, const BitBoard enemyPieces, char color);
    void generatePawnMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorAsInt);

    // king moves
    void generateKingMoves(std::vector<BitMove>& moves, BitBoard kingBoard, uint64_t occupancy);

    int _currentPlayer;
    std::vector<BitMove>    _moves;
    BitBoard _bitboards[e_numBitboards];
    int _bitboardLookup[128];
    Grid* _grid;
    BitBoard _knightBitboards[64];  // knight bitboard
};

