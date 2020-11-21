/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "code_gen.h"

#include "lookup_context.h"
#include "object_output.h"

JumpPointData::JumpPointData( Type type ) : type(type) {
}

FunctionGenImpl::~FunctionGenImpl() {
    assert(llvmFunction==nullptr);
    assert(currentBlock==nullptr);
    assert(builder==nullptr);
}

void FunctionGenImpl::functionEnter(
        String name, StaticType::CPtr returnType, Slice<const ArgumentDeclaration> arguments,
        String file, const SourceLocation &location)
{
    std::cout<<"  Entering function "<<name<<"(";
    bool first=true;
    for( auto &argument : arguments ) {
        if( !first ) {
            std::cout<<", ";
        }
        first = false;
        std::cout<<argument.name<<" : "<<argument.type<<" = "<<argument.lvalueId;
    }

    std::cout<<") -> "<<returnType<<" at "<<location<<"\n";
}

void FunctionGenImpl::functionLeave() {
    std::cout<<"  Leaving function\n";
}

void FunctionGenImpl::returnValue(ExpressionId id) {
    std::cout<<"    Function returning "<<id<<"\n";
}

void FunctionGenImpl::returnValue() {
    std::cout<<"    Function returning didly squat\n";
}

void FunctionGenImpl::conditionalBranch(
        ExpressionId id, StaticType::CPtr type, ExpressionId conditionExpression, JumpPointId elsePoint,
        JumpPointId continuationPoint)
{
    if( id!=ExpressionId() ) {
        std::cout<<"    "<<id<<" "<<type<<" = condition( "<<conditionExpression<<" : false->"<<elsePoint<<", next->"<<
                continuationPoint<<"\n";
    } else {
        std::cout<<"    condition( "<<conditionExpression<<" : false->"<<elsePoint<<", next->"<<continuationPoint<<"\n";
    }
}

void FunctionGenImpl::setConditionClauseResult( ExpressionId id ) {
    std::cout<<"    Condition clause returns "<<id<<"\n";
}

void FunctionGenImpl::setJumpPoint(JumpPointId id, String name) {
    std::cout<<id<<": "<<name<<"\n";
}

void FunctionGenImpl::jump(JumpPointId destination) {
    std::cout<<"    Jump "<<destination<<"\n";
}

void FunctionGenImpl::setLiteral(ExpressionId id, LongEnoughInt value, StaticType::CPtr type) {
    std::cout<<"    "<<id<<" Literal "<<value<<" : "<<type<<"\n";
}

void FunctionGenImpl::setLiteral(ExpressionId id, bool value) {
    std::cout<<"    "<<id<<" Literal "<<(value ? "true" : "false" )<<" : Bool\n";
}

void FunctionGenImpl::allocateStackVar(ExpressionId id, StaticType::CPtr type, String name) {
    std::cout<<"    "<<id<<" new var "<<name<<" : "<<type<<"\n";
}

void FunctionGenImpl::assign( ExpressionId lvalue, ExpressionId rvalue ) {
    std::cout<<"    Assign "<<lvalue<<" <- "<<rvalue<<"\n";
}

void FunctionGenImpl::dereferencePointer( ExpressionId id, StaticType::CPtr type, ExpressionId addr ) {
    std::cout<<"    "<<id<<" dereference "<<addr<<" : "<<type<<"\n";
}

void FunctionGenImpl::truncateInteger(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    std::cout<<"    "<<id<<" truncate "<<source<<" : "<<sourceType<<" => "<<destType<<"\n";
}

void FunctionGenImpl::changeIntegerSign(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    std::cout<<"    "<<id<<" change sign "<<source<<" : "<<sourceType<<" => "<<destType<<"\n";
}

void FunctionGenImpl::expandIntegerSigned(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    std::cout<<"    "<<id<<" sign extend "<<source<<" : "<<sourceType<<" => "<<destType<<"\n";
}

void FunctionGenImpl::expandIntegerUnsigned(
        ExpressionId id, ExpressionId source, StaticType::CPtr sourceType, StaticType::CPtr destType )
{
    std::cout<<"    "<<id<<" unsign extend "<<source<<" : "<<sourceType<<" => "<<destType<<"\n";
}

void FunctionGenImpl::callFunctionDirect(
        ExpressionId id, String name, Slice<const ExpressionId> arguments, StaticType::CPtr returnType )
{
    std::cout<<"    "<<id<<" call "<<name<<"(";
    bool first=true;
    for( auto &argument : arguments ) {
        if( !first )
            std::cout<<", ";
        first = false;
        std::cout<<argument;
    }
    std::cout<<") : "<<returnType<<"\n";
}

void FunctionGenImpl::binaryOperatorPlusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" +(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorPlusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" +(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorMinusUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" -(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorMinusSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" -(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorMultiplyUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" *(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorMultiplySigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" *(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::binaryOperatorDivideUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" /(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" == "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorNotEquals(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" != "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorLessThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" <(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorLessThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" <(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorLessThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" <=(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorLessThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" <=(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorGreaterThanUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" >(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorGreaterThanSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" >(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorGreaterThanOrEqualsUnsigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" >=(U) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorGreaterThanOrEqualsSigned(
            ExpressionId id, ExpressionId left, ExpressionId right, StaticType::CPtr resultType )
{
    std::cout<<"    "<<id<<" "<<left<<" >=(S) "<<right<<" : "<<resultType<<"\n";
}

void FunctionGenImpl::operatorLogicalNot( ExpressionId id, ExpressionId argument ) {
    std::cout<<"    "<<id<<" ! "<<argument<<"\n";
}

void ModuleGenImpl::moduleEnter(
            ModuleId id,
            String name,
            String file,
            size_t line,
            size_t col)
{
    std::cout<<"Start module "<<name<<": "<<id<<" at "<<file<<":"<<line<<":"<<col<<"\n";
}

void ModuleGenImpl::moduleLeave(ModuleId id) {
    std::cout<<"Leave module "<<id<<"\n";
}

void ModuleGenImpl::dump() {
}

void ModuleGenImpl::declareIdentifier(String name, String mangledName, StaticType::CPtr type) {
    std::cout<<"  Forward declare "<<name<<" \""<<mangledName<<"\" as "<<type<<"\n";
}

std::shared_ptr<FunctionGen> ModuleGenImpl::handleFunction() {
    return std::shared_ptr<FunctionGen>( new FunctionGenImpl(this) );
}

ObjectOutput::ObjectOutput(std::filesystem::path outputFile, const char *targetTriplet, ModuleGenImpl &module) {}
