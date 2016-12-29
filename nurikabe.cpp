#include <cstdio>
#include <cstring>
#include <vector>
#include <cassert>

using namespace std;

enum State {
    Unknown = 0,
    White = 1,
    Black = 2,
};

enum Result {
    Change = 0,
    NoChange = 1,
    Impossible = 2,
};

struct Grid {
    int width, height;

    vector<vector<int>> input;
    vector<vector<State>> grid;

    Grid(int width, int height, vector<vector<int>> const& input) : width(width), height(height), input(input) {
        grid.resize(width);
        for (int i = 0; i < width; i++) {
            grid[i].resize(height);
            for (int j = 0; j < height; j++) {
                grid[i][j] = Unknown;
            }
        }
    }

    void dump() {
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                printf("%c", grid[i][j] == Black ? '.' : (grid[i][j] == White ? ' ' : '?'));
            }
            printf("\n");
        }
    }
};

//////////////////////////////// Constraint
//////////////////////////////// No 2x2 black grid

Result no2x2SquareGrid(Grid& g) {
    bool changeMade = false;
    for (int i = 0; i < g.width - 1; i++) {
        for (int j = 0; j < g.height - 1; j++) {
            int numBlack = 0;
            int unknown_i = -1;
            int unknown_j = -1;

            #define check(i, j) if (g.grid[i][j] == Black) { numBlack++; } else if (g.grid[i][j] == Unknown) { unknown_i = i; unknown_j = j; }
            check(i, j)
            check(i+1, j)
            check(i, j+1)
            check(i+1, j+1)

            if (numBlack == 3 && unknown_i != -1) {
                g.grid[unknown_i][unknown_j] = White;
                changeMade = true;
            } else if (numBlack == 4) {
                return Impossible;
            }
        }
    }

    return changeMade ? Change : NoChange;
}

//////////////////////////////// Constraint
//////////////////////////////// Blacks must be connected

int dx[] = {1, -1, 0, 0};
int dy[] = {0, 0, 1, -1};

class FindBlackArticulationPoints {
public:
    struct Data {
        bool visited;
        bool isArticulationPoint;
        bool hasBlackDescendant;
        int lowpoint;
        int depth;
    };
    vector<vector<Data>> data;

    void initialize(Grid& g) {
        data.resize(g.width);
        for (int i = 0; i < g.width; i++) {
            data[i].resize(g.height);
            for (int j = 0; j < g.height; j++) {
                data[i][j].visited = false;
                data[i][j].isArticulationPoint = false;
                data[i][j].hasBlackDescendant = false;
            }
        }
    }

    void dfs(Grid& g, pair<int, int> pos, int depth) {
        Data& dat = data[pos.first][pos.second];
        dat.visited = true;
        dat.hasBlackDescendant = g.grid[pos.first][pos.second] == Black;

        dat.lowpoint = depth;
        dat.depth = depth;

        int numChildren = 0;

        for (int d = 0; d < 4; d++) {
            int x = pos.first + dx[d];
            int y = pos.second + dy[d];
            if (x >= 0 && x < g.width && y >= 0 && y < g.height && g.grid[x][y] != White) {
                if (!data[x][y].visited) {
                    dfs(g, pair<int, int>(x, y), depth + 1);
                    dat.hasBlackDescendant = dat.hasBlackDescendant || data[x][y].hasBlackDescendant;

                    numChildren++;
                    if (depth != 0 && data[x][y].lowpoint >= depth) {
                        dat.isArticulationPoint = true;
                    }
                    dat.lowpoint = min(dat.lowpoint, data[x][y].lowpoint);
                } else {
                    dat.lowpoint = min(dat.lowpoint, data[x][y].depth);
                }
            }
        }

        if (depth == 0 && numChildren >= 2) {
            dat.isArticulationPoint = true;
        }
    }
};

FindBlackArticulationPoints findBlackArticulationPoints;

Result blackMustBeConnected(Grid& g) {
    int black_i = -1;
    int black_j = -1;
    for (int i = 0; i < g.width; i++) {
        for (int j = 0; j < g.height; j++) {
            if (g.grid[i][j] == Black) {
                black_i = i;
                black_j = j;
            }
        }
    }

    // If there are no black cells, we cannot deduce anything.
    if (black_i == -1) {
        return NoChange;
    }

    findBlackArticulationPoints.initialize(g); 
    findBlackArticulationPoints.dfs(g, pair<int, int>(black_i, black_j), 0);

    int numVisited = 0;
    bool hasMadeChange = false;
    for (int i = 0; i < g.width; i++) {
        for (int j = 0; j < g.height; j++) {
            if (findBlackArticulationPoints.data[i][j].visited) {
                numVisited++;
                if (findBlackArticulationPoints.data[i][j].isArticulationPoint &&
                    !findBlackArticulationPoints.data[i][j].hasBlackDescendant &&
                    g.grid[i][j] == Unknown) {
                    g.grid[i][j] = Black;
                    hasMadeChange = true;
                }
            } else if (g.grid[i][j] == Black) {
                // If a black node wasn't visited, then the black nodes aren't connected
                return Impossible;
            }
        }
    }

    return hasMadeChange ? Change : NoChange;
}

//////////////////////////////// Constraint
//////////////////////////////// Correct number of white squares

Result correctNumberOfWhiteSquares(Grid& g) {
    bool hasChange = false;

    int numberSum = 0;
    int numWhiteCells = 0;
    int numBlackCells = 0;
    for (int i = 0; i < g.width; i++) {
        for (int j = 0; j < g.height; j++) {
            if (g.input[i][j] != -1) {
                numberSum += g.input[i][j];
            }
            if (g.grid[i][j] == White) {
                numWhiteCells++;
            } else if (g.grid[i][j] == Black) {
                numBlackCells++;
            }
        }
    }

    int n = g.width * g.height;

    if (numWhiteCells == numberSum) {
        for (int i = 0; i < g.width; i++) {
            for (int j = 0; j < g.height; j++) {
                if (g.grid[i][j] == Unknown) {
                    g.grid[i][j] = Black;
                    hasChange = true;
                }
            }
        }
        return hasChange ? Change : NoChange;
    }

    if (numBlackCells == n - numberSum) {
        for (int i = 0; i < g.width; i++) {
            for (int j = 0; j < g.height; j++) {
                if (g.grid[i][j] == Unknown) {
                    g.grid[i][j] = White;
                    hasChange = true;
                }
            }
        }
        return hasChange ? Change : NoChange;
    }

    if (numWhiteCells > numberSum || numWhiteCells + (n - numWhiteCells - numBlackCells) < numberSum) {
        return Impossible;
    }

    return NoChange;
}

//////////////////////////////// Polyomino utilities

struct Polyomino {
    vector<pair<int, int>> interior;
    vector<pair<int, int>> border;

    void dump() const {
        printf("interior:\n");
        for (auto p : interior) {
            printf("   (%d, %d)\n", p.first, p.second);
        }
        printf("exterior:\n");
        for (auto p : border) {
            printf("   (%d, %d)\n", p.first, p.second);
        }
    }
};

vector<vector<Polyomino>> polyominoCache;

int dxIncludingDiag[] = {1, -1, 0, 0, 1, 1, -1, -1};
int dyIncludingDiag[] = {0, 0, 1, -1, 1, -1, 1, -1};

bool isConnected(vector<pair<int, int>> const& squares) {
    assert(squares.size() > 0);
    int minX = squares[0].first;
    int maxX = squares[0].first;
    int minY = squares[0].second;
    int maxY = squares[0].second;
    for (int i = 1; i < squares.size(); i++) {
        minX = min(squares[i].first, minX);
        maxX = max(squares[i].first, maxX);
        minY = min(squares[i].second, minY);
        maxY = max(squares[i].second, maxY);
    }

    vector<vector<bool>> visited;
    vector<vector<bool>> exists;
    visited.resize(maxX - minX + 1);
    exists.resize(maxX - minX + 1);
    for (int i = 0; i < maxX - minX + 1; i++) {
        visited[i].resize(maxY - minY + 1);
        exists[i].resize(maxY - minY + 1);
    }

    for (auto p : squares) {
        exists[p.first - minX][p.second - minY] = true;
    }

    // bfs
    visited[squares[0].first - minX][squares[0].second - minY] = true;
    vector<pair<int, int>> queue;
    queue.push_back(squares[0]);
    int index = 0;
    while (index < queue.size()) {
        pair<int, int> thisSqr = queue[index];
        index++;

        for (int d = 0; d < 8; d++) {
            int x = thisSqr.first + dxIncludingDiag[d];
            int y = thisSqr.second + dyIncludingDiag[d];
            if (x >= minX && x <= maxX && y >= minY && y <= maxY && exists[x - minX][y - minY]) {
                if (!visited[x - minX][y - minY]) {
                    visited[x - minX][y - minY] = true;
                    queue.push_back(pair<int, int>(x, y));
                }
            }
        }
    }

    return queue.size() == squares.size();
}

vector<pair<int, int>> getBorderOfPolyomino(vector<pair<int, int>> squares) {
    assert(squares.size() > 0);
    int minX = squares[0].first;
    int maxX = squares[0].first;
    int minY = squares[0].second;
    int maxY = squares[0].second;
    for (int i = 1; i < squares.size(); i++) {
        minX = min(squares[i].first, minX);
        maxX = max(squares[i].first, maxX);
        minY = min(squares[i].second, minY);
        maxY = max(squares[i].second, maxY);
    }

    vector<vector<bool>> exists;
    exists.resize(maxX - minX + 1);
    for (int i = 0; i < maxX - minX + 1; i++) {
        exists[i].resize(maxY - minY + 1);
    }

    for (auto p : squares) {
        exists[p.first - minX][p.second - minY] = true;
    }

    vector<pair<int, int>> result;
    for (int i = minX - 1; i <= maxX + 1; i++) {
        for (int j = minY - 1; j <= maxY + 1; j++) {
            bool isBorder = false;
            for (int d = 0; d < 4; d++) {
                int x = i + dx[d];
                int y = j + dy[d];
                if (
                    !(i >= minX && i <= maxX && j >= minY && j <= maxY && exists[i - minX][j - minY]) &&
                    x >= minX && x <= maxX && y >= minY && y <= maxY && exists[x - minX][y - minY]) {
                    isBorder = true;
                    break;
                }
            }
            if (isBorder) {
                result.push_back(pair<int, int>(i, j));
            }
        }
    }
    return result;
}

vector<Polyomino> const& getPolyominos(int size) {
    while (polyominoCache.size() < size + 1) {
        int n = polyominoCache.size();
        vector<Polyomino> polys;

        if (n == 0) {
            Polyomino p;
            p.border.push_back(pair<int, int>(0, 0));
            polys.push_back(p);
        } else {
            for (Polyomino p : polyominoCache[n-1]) {
                for (pair<int, int> extra : p.border) {
                    Polyomino q;
                    q.interior = p.interior;
                    q.interior.push_back(extra);
                    q.border = getBorderOfPolyomino(q.interior);
                    if (isConnected(q.border)) {
                        polys.push_back(std::move(q));
                    }
                }
            }
        }

        polyominoCache.push_back(std::move(polys));
    }

    return polyominoCache[size];
}

//////////////////////////////// Constraint
//////////////////////////////// Islands are correct size

bool canUsePolyominoAt(Grid const& g, Polyomino const& poly, int i, int j) {
    for (pair<int, int> p : poly.interior) {
        int x = p.first + i;
        int y = p.second + j;
        if (!(x >= 0 && x < g.width && y >= 0 && y < g.height && g.grid[x][y] != Black)) {
            return false;
        }
    }

    for (pair<int, int> p : poly.border) {
        int x = p.first + i;
        int y = p.second + j;
        if (!(x < 0 || x >= g.width || y < 0 || y >= g.height || g.grid[x][y] != White)) {
            return false;
        }
    }

    return true;
}

Result islandsAreCorrectSize(Grid& g) {
    vector<vector<int>> intermediate, thisPoly;
    intermediate.resize(g.width);
    thisPoly.resize(g.width);
    for (int i = 0; i < g.width; i++) {
        intermediate[i].resize(g.height);
        thisPoly[i].resize(g.height);
    }

    bool hasChange = false;

    for (int i = 0; i < g.width; i++) {
        for (int j = 0; j < g.height; j++) {
            if (g.input[i][j] != -1) {
                int minX = max(0, i - g.input[i][j]);
                int maxX = min(g.width - 1, i + g.input[i][j]);
                int minY = max(0, j - g.input[i][j]);
                int maxY = min(g.height - 1, j + g.input[i][j]);
                for (int x = minX; x <= maxX; x++) {
                    for (int y = minY; y <= maxY; y++) {
                        intermediate[x][y] = 0;
                    }
                }

                const int WHITE_BIT = 1;
                const int BLACK_BIT = 2;

                vector<Polyomino> const& polyominos = getPolyominos(g.input[i][j]);
                for (Polyomino const& polyomino : polyominos) {
                    if (canUsePolyominoAt(g, polyomino, i, j)) {
                        for (int x = minX; x <= maxX; x++) {
                            for (int y = minY; y <= maxY; y++) {
                                thisPoly[x][y] = false;
                            }
                        }
                        for (pair<int, int> p : polyomino.interior) {
                            intermediate[i + p.first][j + p.second] |= WHITE_BIT;
                            thisPoly[i + p.first][j + p.second] = true;
                        }
                        for (pair<int, int> p : polyomino.border) {
                            int x = i + p.first;
                            int y = j + p.second;
                            if (x >= 0 && x < g.width && y >= 0 && y < g.width) {
                                intermediate[x][y] |= BLACK_BIT;
                                thisPoly[x][y] = true;
                            }
                        }
                        for (int x = minX; x <= maxX; x++) {
                            for (int y = minY; y <= maxY; y++) {
                                if (!thisPoly[x][y]) {
                                    intermediate[x][y] |= WHITE_BIT | BLACK_BIT;
                                }
                            }
                        }
                    }
                }

                for (int x = minX; x <= maxX; x++) {
                    for (int y = minY; y <= maxY; y++) {
                        if (intermediate[x][y] == 0) {
                            return Impossible;
                        } else if (intermediate[x][y] == BLACK_BIT) {
                            if (g.grid[x][y] == Unknown) {
                                g.grid[x][y] = Black;
                                hasChange = true;
                            } else if (g.grid[x][y] == White) {
                                assert(false);
                            }
                        } else if (intermediate[x][y] == WHITE_BIT) {
                            if (g.grid[x][y] == Unknown) {
                                g.grid[x][y] = White;
                                hasChange = true;
                            } else if (g.grid[x][y] == Black) {
                                assert(false);
                            }
                        }
                    }
                }
            }
        }
    }

    return hasChange ? Change : NoChange;
}

//////////////////////////////// High-level solving logic

auto constraintFunctions = {no2x2SquareGrid, blackMustBeConnected, correctNumberOfWhiteSquares, islandsAreCorrectSize};

void iterateAndSolve(Grid g, vector<vector<vector<State>>>& solutions) {
    while (true) {
        // Check if g is solved
        bool isSolved = true;
        int unsolved_i, unsolved_j;
        for (int i = 0; i < g.width; i++) {
            for (int j = 0; j < g.height; j++) {
                if (g.grid[i][j] == Unknown) {
                    isSolved = false;
                    unsolved_i = i;
                    unsolved_j = j;
                    goto doneLoop;
                }
            }
        }
        doneLoop:

        if (isSolved) {
            for (auto func : constraintFunctions) {
                Result res = func(g);
                assert(res != Change);
                if (res == Impossible) {
                    return;
                }
            }
            solutions.push_back(g.grid);
            return;
        } else {
            bool hasChange = false;
            for (auto func : constraintFunctions) {
                Result res = func(g);
                if (res == Impossible) {
                    return;
                }
                if (res == Change) {
                    hasChange = true;
                }
            }

            if (!hasChange) {
                g.grid[unsolved_i][unsolved_j] = White;
                iterateAndSolve(g, solutions);

                g.grid[unsolved_i][unsolved_j] = Black;
                iterateAndSolve(g, solutions);

                return;
            }
        }
    }
}

vector<vector<vector<State>>> solve(vector<vector<int>> input) {
    int width = input.size();
    assert(width > 0);
    int height = input[0].size();
    assert(height > 0);

    for (int i = 0; i < width; i++) {
        assert(input[i].size() == height);
    }

    Grid g(width, height, input);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (g.input[i][j] != -1) {
                g.grid[i][j] = White;
            }
        }
    }

    vector<vector<vector<State>>> solutions;
    iterateAndSolve(g, solutions);

    return solutions;
}

void dumpSolution(vector<vector<int>> const& input, vector<vector<State>> const& soln) {
    for (int j = 0; j < soln[0].size(); j++) {
        for (int i = 0; i < soln.size(); i++) {
            if (soln[i][j] == Black) {
                assert(input[i][j] == -1);
                printf(".");
            } else {
                if (input[i][j] == -1) {
                    printf(" ");
                } else {
                    printf("%d", input[i][j]);
                }
            }
        }
        printf("\n");
    }
}

int main() {
    int width, height;
    scanf("%d", &width);
    scanf("%d", &height);

    vector<vector<int>> input;
    input.resize(width);
    for (int i = 0; i < width; i++) {
        input[i].resize(height);
    }

    char s[10];

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            scanf("%s", s);
            if (s[0] == '.') {
                input[i][j] = -1;
            } else {
                input[i][j] = atoi(s);
            }
        }
    }

    auto allSolutions = solve(input);
    printf("%d solution(s)\n", (int)allSolutions.size());
    for (auto solution : allSolutions) {
        dumpSolution(input, solution);
        printf("\n");
    }
}

