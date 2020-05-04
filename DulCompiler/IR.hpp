//
//  IR.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef IR_hpp
#define IR_hpp

#include "CFG.hpp"
#include <vector>
struct IROP{
    int lineno, linepos;
    enum types{};
    types type;
    int first_arg;
    int second_arg;
    int third_arg;
};

class ContextedFunctionIR:public ContextedAST{
    std::vector<IROP> ops;
    uint8_t * static_vals;
    int stat_size;
    int stat_cap;
};

#endif /* IR_hpp */
