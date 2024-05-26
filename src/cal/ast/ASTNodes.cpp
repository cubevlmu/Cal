#include "ASTNodes.hpp"

namespace cal {
    const char* ASTBase::NumTypeChar[] = {
        "Float", "Double", "Int", "Long",
        "NotANumber", "ERR_TP",
        "FunctionReturn", "VariableReturn",
        "VOID"
    };

}