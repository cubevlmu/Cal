// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Type.hpp"

#include <utility>
#include "neo/compiler/DebugOutput.hpp"

namespace neo {

    ASTTypeNode::ASTTypeNode(std::string type)
        : ASTNode(kType)
        , typeStr {std::move(type)}
    {

    }

    ASTTypeNode::~ASTTypeNode() {

    }

    void ASTTypeNode::debugPrint(NDebugOutput &output) {
        ASTNode::debugPrint(output);
        output.writeLine("\t   |- Type: {}", typeStr);
    }


    ASTArrayType::ASTArrayType(std::string typeStr, bool isReceiver, std::initializer_list<int> size)
        : ASTTypeNode(std::move(typeStr))
        , isReceiver {isReceiver}
        , size {size}
    {

    }

    ASTArrayType::~ASTArrayType() {
        size.clear();
    }

    void ASTArrayType::debugPrint(NDebugOutput &output) {
        ASTTypeNode::debugPrint(output);
        output.writeLine("\t   |- isReceiver: {}", isReceiver);
        output.writeLine("\t   |- dimession: {}", dimenssion);
        output.writeLine("\t   |- size: ");
        psize idx = 0;
        for (auto& s : size) {
            output.writeLine("\t   |- [{}] {}", idx, s);
            idx++;
        }
    }


    ASTPointerType::ASTPointerType(std::string typeStr)
        : ASTTypeNode(std::move(typeStr))
    {

    }

    ASTPointerType::~ASTPointerType() {

    }

}