//
//  main.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "AST.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    AstNode * p = AstNode::parseFile("example.dul");
    p->setNameScope(LayoutType::createNamespace());
    p->inferTypes();
    std::ofstream a("main.ast");
    p->print(0, a);
    return 0;
}
