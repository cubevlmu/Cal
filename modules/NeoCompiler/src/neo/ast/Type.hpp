/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "Base.hpp"
#include <nbase/memory/Memory.hpp>

namespace neo
{
    class ASTTypeNode : public ASTNode
    {
    public:
        ASTTypeNode(String type);
        ~ASTTypeNode() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        String typeStr;
    };

    class ASTArrayType : public ASTTypeNode
    {
    public:
        ASTArrayType(String typeStr, bool isReceiver, std::initializer_list<int> size);
        ~ASTArrayType() override;

    public:
        void debugPrint(NDebugOutput &output) override;

    public:
        bool isReceiver;
        i32 dimenssion;
        Vector<i32> size;
    };

    class ASTPointerType : public ASTTypeNode
    {
    public:
        ASTPointerType(String typeStr);
        ~ASTPointerType() override;

    public:
        void debugPrint(NDebugOutput &output) override;
    };
}