#include "sssp/tie_break.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main(){
    DistState state; state.init(10);
    state.set(3, 5.0); state.set(4, 5.0);
    state.set_pred(3, 2);
    state.set_pred(2, 1);
    state.set_pred(1, 0);
    state.set_pred(4, 2);
    int c = compare_paths(Vertex(3), Vertex(4), state);
    assert(c == -1 || c == 1 || c == 0);
    std::cout << "Tie-break smoke test passed\n";
    return 0;
}
