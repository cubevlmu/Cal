/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Base.hpp"

#include "neo/compiler/DebugOutput.hpp"
#include "Base.hpp"

namespace neo
{

    static const char *s_ASTTypeStrings[] = {
        "kUnknown",
        "kStatement",
        "kDeclaration",
        "kType",
        "kTypeArray",
        "kTypePointer",
        "kExpression",
        "kInitializer"};
    StringView getTypeString(ASTType type)
    {
        return s_ASTTypeStrings[(int)type];
    }

    static const char *s_StmtKindStrings[] = {
        "kUnknown",
        "kError",
        "kExpression",
        "kCompound",
        "kIf",
        "kWhile",
        "kFor",
        "kForeach",
        "kReturn",
        "kBreak",
        "kContinue",
        "kImport",
        "kDecl",
        "kExpr",
        "kTry",
        "kCatch",
        "kThrow",
        "kInitial"};
    StringView getTypeString(StmtKind type)
    {
        return s_StmtKindStrings[(int)type];
    }

    static const char *s_ExprKindStrings[] = {
        "kUnknown",
        "kError",
        "kNumberLit",
        "kBoolLit",
        "kBinary",
        "kUnary",
        "kFuncCall",
        "kMemberAccess",
        "kVar",
        "kCast",
        "kNew",
        "kStringLit",
        "kCharLit",
        "kIdent",
        "kNull",
        "kLambda",
        "kThis",
        "kSuper",
        "kArrayLit",
        "kSubscript",
        "kPostfix",
        "kCond",
        "kComma"};
    StringView getTypeString(ExprKind type)
    {
        return s_ExprKindStrings[(int)type];
    }

    static const char *s_DeclKindStrings[] = {
        "kUnknown",
        "kError",
        "kGenericParam",
        "kImport",
        "kVar",
        "kFunc",
        "kClass",
        "kField",
        "kStruct",
        "kModule",
        "kInterface",
        "kEnum",
        "kTopLevelDecls"};
    StringView getTypeString(DeclKind type)
    {
        return s_DeclKindStrings[(int)type];
    }

    static void writeAttribute(NSerializer *s, Attribute *attribute)
    {
        s->write(attribute->name);
        s->write(attribute->arguments.size());
        for (auto *item : attribute->arguments)
        {
            item->write(s);
        }
    }

    void ASTNode::debugPrint(NDebugOutput &out)
    {
        out.printItem("type", getTypeString(m_type));
        out.printItem("loc", m_loc.toString());
    }

    void ASTNode::write(NSerializer *s)
    {
        // ASTType
        s->write((i32)m_type);
        // SourceLoc
        m_loc.write(s);
    }

    void ASTNode::read(NSerializer *s)
    {
        // ASTType
        s->read(m_type);
        // SourceLoc
        m_loc.read(s);
        // TODO create instance
    }

    ASTDecl::~ASTDecl()
    {
        for (auto *ptr : attributes)
        {
            neo::deletePtr(ptr);
        }
        attributes.clear();
    }

    void ASTDecl::write(NSerializer *s)
    {
        ASTNode::write(s);

        // Modifier
        s->write(modifier.isInternal);
        s->write(modifier.isPrivate);
        s->write(modifier.isStatic);
        s->write(modifier.isConst);
        s->write(modifier.isFinal);
        s->write(modifier.isInline);
        s->write(modifier.isProtected);
        s->write(modifier.isVirtual);
        s->write(modifier.isOverride);
        s->write(modifier.isImpl);

        // isMarkedExport
        s->write(isMarkedExport);

        // attributes
        s->write(attributes.size());
        for (auto *attribute : attributes)
        {
            writeAttribute(s, attribute);
        }
    }

    void ASTDecl::read(NSerializer *s)
    {
        ASTNode::read(s);
    }

    void ASTDecl::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);

        out.printItem("declKind", getTypeString(m_kind));
        out.printItem("exported", isMarkedExport ? "true" : "false");

        out.printItem("modifier", "static={}, final={}, const={}, private={}, protected={}, internal={}, inline={}, virtual={}, override={}, impl={}",
                      modifier.isStatic, modifier.isFinal, modifier.isConst,
                      modifier.isPrivate, modifier.isProtected,
                      modifier.isInternal, modifier.isInline, modifier.isVirtual,
                      modifier.isOverride, modifier.isImpl);

        out.printChildren("attributes", attributes);
    }

    ASTModifier::ASTModifier() noexcept
        : isStatic(false), isFinal(false), isConst(false),
          isPrivate(false), isProtected(false), isInternal(false), isInline(false),
          isVirtual(false), isOverride(false), isImpl(false) {}

    ASTModifier::ASTModifier(bool s, bool f, bool c, bool priv, bool prot, bool inter, bool inl, bool vt, bool over, bool impl) noexcept
        : isStatic(s), isFinal(f), isConst(c),
          isPrivate(priv), isProtected(prot), isInternal(inter), isInline(inl),
          isVirtual(vt), isOverride(over), isImpl(impl) {}

    ASTModifier::ASTModifier(const ASTModifier &other) noexcept
        : isStatic(other.isStatic), isFinal(other.isFinal), isConst(other.isConst),
          isPrivate(other.isPrivate), isProtected(other.isProtected),
          isInternal(other.isInternal), isInline(other.isInline),
          isVirtual(other.isVirtual), isOverride(other.isOverride),
          isImpl(other.isImpl) {}

    ASTModifier &ASTModifier::operator=(const ASTModifier &other) noexcept
    {
        if (this != &other)
        {
            isStatic = other.isStatic;
            isFinal = other.isFinal;
            isConst = other.isConst;
            isPrivate = other.isPrivate;
            isProtected = other.isProtected;
            isInternal = other.isInternal;
            isInline = other.isInline;
            isVirtual = other.isVirtual;
            isOverride = other.isOverride;
            isImpl = other.isImpl;
        }
        return *this;
    }

    ASTModifier::ASTModifier(ASTModifier &&other) noexcept
        : isStatic(other.isStatic), isFinal(other.isFinal), isConst(other.isConst),
          isPrivate(other.isPrivate), isProtected(other.isProtected),
          isInternal(other.isInternal), isInline(other.isInline),
          isVirtual(other.isVirtual), isOverride(other.isOverride), isImpl(other.isImpl)
    {
    }

    ASTModifier &ASTModifier::operator=(ASTModifier &&other) noexcept
    {
        if (this != &other)
        {
            isStatic = other.isStatic;
            isFinal = other.isFinal;
            isConst = other.isConst;
            isPrivate = other.isPrivate;
            isProtected = other.isProtected;
            isInternal = other.isInternal;
            isInline = other.isInline;
            isVirtual = other.isVirtual;
            isOverride = other.isOverride;
            isImpl = other.isImpl;
        }
        return *this;
    }

    bool ASTModifier::operator==(const ASTModifier &other) const noexcept
    {
        return isStatic == other.isStatic &&
               isFinal == other.isFinal &&
               isConst == other.isConst &&
               isPrivate == other.isPrivate &&
               isProtected == other.isProtected &&
               isInternal == other.isInternal &&
               isInline == other.isInline &&
               isVirtual == other.isVirtual &&
               isOverride == other.isOverride &&
               isImpl == other.isImpl;
    }

    bool ASTModifier::operator!=(const ASTModifier &other) const noexcept
    {
        return !(*this == other);
    }

    void ASTStmt::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);

        out.printItem("stmtKind", getTypeString(m_kind));
    }

    void ASTExpr::debugPrint(NDebugOutput &out)
    {
        ASTNode::debugPrint(out);

        out.printItem("exprKind", getTypeString(m_kind));
    }

    void Attribute::debugPrint(NDebugOutput &out)
    {
        out.beginObject("Attribute");

        out.printItem("name", name);

        // arguments
        if (arguments.empty())
        {
            out.printItem("arguments", "<empty>");
        }
        else
        {
            out.printChildren("arguments", arguments);
        }

        out.endObject();
    }
}
