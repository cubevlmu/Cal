/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <nbase/memory/Memory.hpp>
#include <nbase/base/Serializer.hpp>

#include "neo/diagnose/SourceLoc.hpp"

#include <cstddef>
#include <new>

namespace neo
{

    class NDebugOutput;
    class NSerializer;

    enum ASTType
    {
        kUnknown,
        kStatment,
        kDeclaration,
        kType,
        kTypeArray,
        kTypePointer,
        kUnused1
    };
    StringView getTypeString(ASTType);

    struct Attribute
    {
        String name;
        Vector<class ASTExpr *> arguments;

        void debugPrint(NDebugOutput &out);

        static void *operator new(std::size_t size)
        {
            return neo::allocAligned(size, alignof(std::max_align_t));
        }

        static void operator delete(void *ptr) noexcept
        {
            neo::free(ptr);
        }

        static void *operator new(std::size_t size, std::align_val_t align)
        {
            return neo::allocAligned(size, static_cast<psize>(align));
        }

        static void operator delete(void *ptr, std::align_val_t) noexcept
        {
            neo::free(ptr);
        }

        static void *operator new(std::size_t, void *ptr) noexcept
        {
            return ptr;
        }

        static void operator delete(void *, void *) noexcept
        {
        }
    };

    class ASTNode : public ISerializable
    {
    public:
        explicit ASTNode(ASTType type)
            : m_type{type}
        {
        }
        ~ASTNode() override = default;

    public:
        virtual void debugPrint(NDebugOutput &output);

        void read(NSerializer *s) override;
        void write(NSerializer *s) override;

        ASTType getType() const
        {
            return m_type;
        }

        SourceLoc m_loc;

        static void *operator new(std::size_t size)
        {
            return neo::allocAligned(size, alignof(std::max_align_t));
        }

        static void operator delete(void *ptr) noexcept
        {
            neo::free(ptr);
        }

        static void *operator new(std::size_t size, std::align_val_t align)
        {
            return neo::allocAligned(size, static_cast<psize>(align));
        }

        static void operator delete(void *ptr, std::align_val_t) noexcept
        {
            neo::free(ptr);
        }

        static void *operator new(std::size_t, void *ptr) noexcept
        {
            return ptr;
        }

        static void operator delete(void *, void *) noexcept
        {
        }

    private:
        ASTType m_type;
    };

    enum class StmtKind
    {
        kUnknown,
        kExpression,
        kCompound,
        kIf,
        kWhile,
        kFor,
        kForeach,
        kReturn,
        kBreak,
        kContinue,
        kImport,
        kDecl,
        kExpr,
        kTry,
        kCatch,
        kThrow
    };
    StringView getTypeString(StmtKind);
    class ASTStmt *createStmt(StmtKind);

    class ASTStmt : public ASTNode
    {
    public:
        explicit ASTStmt(StmtKind kind)
            : ASTNode(ASTType::kStatment), m_kind{kind}
        {
        }
        ~ASTStmt() override = default;

    public:
        virtual void debugPrint(NDebugOutput &output) override;

    public:
        NE_FORCE_INLINE StmtKind getStmtKind() const
        {
            return m_kind;
        }
        virtual void visit(class ASTVisitor &visitor) {}

    private:
        const StmtKind m_kind;
    };

    enum class ExprKind
    {
        kUnknown,
        kNumberLit,
        kBoolLit,
        kBinary,
        kUnary,
        kFuncCall,
        kMemberAccess,
        kVar,
        kCast,
        kNew,
        kStringLit,
        kCharLit,
        kIdent,
        kNull,
        kLambda,
        kThis,
        kSuper,
        kArrayLit,
        kSubscript,
        kPostfix,
        kCond,
        kComma
    };
    StringView getTypeString(ExprKind);
    class ASTExpr *createExpr(ExprKind);

    class ASTExpr : public ASTStmt
    {
    public:
        explicit ASTExpr(ExprKind kind)
            : ASTStmt(StmtKind::kExpression), m_kind{kind}
        {
        }
        ~ASTExpr() override = default;

    public:
        NE_FORCE_INLINE ExprKind getExprKind() const
        {
            return m_kind;
        }

    public:
        virtual void debugPrint(NDebugOutput &out) override;

    private:
        ExprKind m_kind;
    };

    template <typename T>
    class SingletonExpr
    {
    public:
        static T *getInstance()
        {
            static T instance{};
            return &instance;
        }

    protected:
        SingletonExpr() {}
        ~SingletonExpr() {}
    };

    enum class DeclKind
    {
        kUnknown,
        kVar,
        kFunc,
        kClass,
        kField,
        kStruct,
        kModule,
        kInterface,
        kEnum,
        kTopLevelDecls
    };
    StringView getTypeString(DeclKind);
    class ASTDecl *createDecl(DeclKind);

    struct ASTModifier
    {
        bool isStatic : 1 = false;
        bool isFinal : 1 = false;
        bool isConst : 1 = false;
        bool isPrivate : 1 = false;
        bool isProtected : 1 = false;
        bool isInternal : 1 = false;
        bool isInline : 1 = false;

        ASTModifier() noexcept;
        ASTModifier(bool s, bool f, bool c, bool priv, bool prot, bool inter, bool inl) noexcept;

        ASTModifier(const ASTModifier &other) noexcept;
        ASTModifier &operator=(const ASTModifier &other) noexcept;

        ASTModifier(ASTModifier &&other) noexcept;
        ASTModifier &operator=(ASTModifier &&other) noexcept;

        bool operator==(const ASTModifier &other) const noexcept;
        bool operator!=(const ASTModifier &other) const noexcept;
    };

    class ASTDecl : public ASTNode
    {
    public:
        explicit ASTDecl(DeclKind kind)
            : ASTNode(ASTType::kDeclaration), m_kind{kind}
        {
        }
        ~ASTDecl() override;

    public:
        NE_FORCE_INLINE DeclKind getDeclKind() const
        {
            return m_kind;
        }

        void read(NSerializer *s) override;
        void write(NSerializer *s) override;

        virtual void debugPrint(NDebugOutput &out) override;

    public:
        bool isMarkedExport;
        Vector<Attribute *> attributes;

        ASTModifier modifier;

    private:
        DeclKind m_kind;
    };

}
