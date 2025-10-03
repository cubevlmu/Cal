// Created by cubevlmu on 2025/10/3.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Base.hpp"

#include "neo/compiler/DebugOutput.hpp"
#include "Base.hpp"

namespace neo {

    static const char* s_ASTTypeStrings[] = {  
        "kUnknown",
        "kStatment",
        "kDeclaration",
        "kType",
        "kTypeArray",
        "kTypePointer",
        "_________"
    };
    std::string_view getTypeString(ASTType type) {
        return s_ASTTypeStrings[(int)type];
    }


    static const char* s_StmtKindStrings[] = {
        "kUnknown",
        "kExpression",
        "kCompound",
        "kIf",
        "kWhile",
        "kFor",
        "kForeach",
        "kReturn",
        "kBreak",
        "kContinue",
        "kImport"
    };
    std::string_view getTypeString(StmtKind type) {
        return s_StmtKindStrings[(int)type];
    }


    static const char* s_ExprKindStrings[] = {
        "kUnknown",
        "kNumberLit",
        "kBoolLit",
        "kBinary",
        "kUnary",
        "kFuncCall",
        "kMemberAccess",
        "kVar",
        "kCast",
        "kNew"
    };
    std::string_view getTypeString(ExprKind type) {
        return s_ExprKindStrings[(int)type];
    }
    
    
    static const char* s_DeclKindStrings[] = {
        "kUnknown",
        "kVar",
        "kFunc",
        "kClass",
        "kField",
        "kStruct",
        "kModule",
        "kInterface",
        "kEnum",
        "kTopLevelDecls"
    };
    std::string_view getTypeString(DeclKind type) {
        return s_DeclKindStrings[(int)type];
    }


    static void writeAttribute(NSerializer* s, Attribute* attribute) {
        s->write(attribute->name);
        s->write(attribute->arguments.size());
        for (auto* item : attribute->arguments) {
            item->write(s);
        }
    }


    void ASTNode::debugPrint(NDebugOutput& output) {
        output.writeLine("\t|- {}", getTypeString(m_type));
    }
    

    void ASTNode::write(NSerializer* s)
    {
        // ASTType
        s->write((i32)m_type);
        // SourceLoc
        m_loc.write(s);
    }


    void ASTNode::read(NSerializer* s)
    {
        // ASTType
        s->read(m_type);
        // SourceLoc
        m_loc.read(s);
        //TODO create instance
    }


    ASTDecl::~ASTDecl() {
        for (auto* ptr : attributes) {
            delete ptr;
        }
        attributes.clear();
    }

    void ASTDecl::write(NSerializer *s) {
        ASTNode::write(s);

        // Modifier
        s->write(modifier.isInternal);
        s->write(modifier.isPrivate);
        s->write(modifier.isStatic);
        s->write(modifier.isConst);
        s->write(modifier.isFinal);
        s->write(modifier.isInline);
        s->write(modifier.isProtected);

        // isMarkedExport
        s->write(isMarkedExport);

        // attributes
        s->write(attributes.size());
        for (auto* attribute : attributes) {
            writeAttribute(s, attribute);
        }
    }

    void ASTDecl::read(NSerializer *s) {
        ASTNode::read(s);
    }

    void ASTDecl::debugPrint(NDebugOutput &output) {
        ASTNode::debugPrint(output);
        output.writeLine("\t|- DeclType: {}", getTypeString(m_kind));
    }


    ASTModifier::ASTModifier() noexcept
            : isStatic(false), isFinal(false), isConst(false),
              isPrivate(false), isProtected(false), isInternal(false), isInline(false) {}

    ASTModifier::ASTModifier(bool s, bool f, bool c, bool priv, bool prot, bool inter, bool inl) noexcept
            : isStatic(s), isFinal(f), isConst(c),
              isPrivate(priv), isProtected(prot), isInternal(inter), isInline(inl) {}

    ASTModifier::ASTModifier(const ASTModifier& other) noexcept
            : isStatic(other.isStatic), isFinal(other.isFinal), isConst(other.isConst),
              isPrivate(other.isPrivate), isProtected(other.isProtected),
              isInternal(other.isInternal), isInline(other.isInline) {}

    ASTModifier& ASTModifier::operator=(const ASTModifier& other) noexcept {
        if (this != &other) {
            isStatic = other.isStatic;
            isFinal = other.isFinal;
            isConst = other.isConst;
            isPrivate = other.isPrivate;
            isProtected = other.isProtected;
            isInternal = other.isInternal;
            isInline = other.isInline;
        }
        return *this;
    }

    ASTModifier::ASTModifier(ASTModifier&& other) noexcept
            : isStatic(other.isStatic), isFinal(other.isFinal), isConst(other.isConst),
              isPrivate(other.isPrivate), isProtected(other.isProtected),
              isInternal(other.isInternal), isInline(other.isInline) {
        // 对 bool 来说不需要清空 other
    }

    ASTModifier& ASTModifier::operator=(ASTModifier&& other) noexcept {
        if (this != &other) {
            isStatic = other.isStatic;
            isFinal = other.isFinal;
            isConst = other.isConst;
            isPrivate = other.isPrivate;
            isProtected = other.isProtected;
            isInternal = other.isInternal;
            isInline = other.isInline;
        }
        return *this;
    }

    bool ASTModifier::operator==(const ASTModifier& other) const noexcept {
        return isStatic == other.isStatic &&
               isFinal == other.isFinal &&
               isConst == other.isConst &&
               isPrivate == other.isPrivate &&
               isProtected == other.isProtected &&
               isInternal == other.isInternal &&
               isInline == other.isInline;
    }

    bool ASTModifier::operator!=(const ASTModifier& other) const noexcept {
        return !(*this == other);
    }
}