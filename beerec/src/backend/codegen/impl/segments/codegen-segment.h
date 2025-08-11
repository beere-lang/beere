#ifndef CODEGEN_SEGMENT_H
#define CODEGEN_SEGMENT_H

#include "../../codegen.h"

char* generate_segment(CodeGen* codegen, SegmentNode* node, Type* type);
void unuse_segment_registers(SegmentNode* node);

#endif