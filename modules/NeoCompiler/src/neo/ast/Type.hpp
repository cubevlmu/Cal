// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include "Base.hpp"
#include <unordered_map>

namespace neo {

    class ASTTypeNode : public ASTNode
    {
    public:
        ASTTypeNode(std::string type);
        ~ASTTypeNode() override;

    public:
        void debugPrint(NDebugOutput& output) override;

    public:
        std::string typeStr;
    };


    class ASTArrayType : public ASTTypeNode
    {
    public:
        ASTArrayType(std::string typeStr, bool isReceiver, std::initializer_list<int> size);
        ~ASTArrayType() override;

    public:
        void debugPrint(NDebugOutput& output) override;

    public:
        bool isReceiver;
        i32 dimenssion;
        std::vector<i32> size;
    };


    class ASTPointerType : public ASTTypeNode
    {
    public:
        ASTPointerType(std::string typeStr);
        ~ASTPointerType() override;
    };
}