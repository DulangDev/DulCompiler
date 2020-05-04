//
//  main.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include <iostream>
#include "AST.hpp"
int main(int argc, const char * argv[]) {
    // insert code here...
    AstNode::parseFile("example.dul")->print(0);
    return 0;
}
