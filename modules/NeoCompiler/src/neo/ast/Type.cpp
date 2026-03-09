/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Type.hpp"

#include <utility>
#include "neo/compiler/DebugOutput.hpp"

namespace neo
{
    ASTTypeNode::ASTTypeNode(String type)
        : ASTNode(kType), typeStr{std::move(type)}
    {
    }

    ASTTypeNode::~ASTTypeNode()
    {
    }

    void ASTTypeNode::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);

        out.beginObject("TypeNode");
        out.printItem("type", typeStr);
        out.endObject();
    }

    ASTArrayType::ASTArrayType(String typeStr, bool isReceiver, std::initializer_list<int> size)
        : ASTTypeNode(std::move(typeStr)), isReceiver{isReceiver}, size{size}
    {
    }

    ASTArrayType::~ASTArrayType()
    {
        size.clear();
    }

    void ASTArrayType::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);
        out.beginObject("ArrayType");

        out.printItem("type", typeStr);
        out.printItem("isReceiver", isReceiver);
        out.printItem("dimension", dimenssion);
        out.printArray("size", size, [](NDebugOutput &o, i32 i)
                       { o.printArrayItem(i); });

        out.endObject();
    }

    ASTPointerType::ASTPointerType(String typeStr)
        : ASTTypeNode(std::move(typeStr))
    {
    }

    ASTPointerType::~ASTPointerType()
    {
    }

    void ASTPointerType::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);
        out.beginObject("PointerType");

        out.printItem("type", typeStr);

        out.endObject();
    }

}