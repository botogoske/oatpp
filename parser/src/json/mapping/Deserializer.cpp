/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "./Deserializer.hpp"

#include "../Utils.hpp"
#include "../../../../../oatpp/core/src/utils/ConversionUtils.hpp"

namespace oatpp { namespace parser { namespace json { namespace mapping {

const char* const Deserializer::ERROR_PARSER_OBJECT_SCOPE_OPEN = "'{' - expected";
const char* const Deserializer::ERROR_PARSER_OBJECT_SCOPE_CLOSE = "'}' - expected";
const char* const Deserializer::ERROR_PARSER_OBJECT_SCOPE_UNKNOWN_FIELD = "Unknown field";
const char* const Deserializer::ERROR_PARSER_OBJECT_SCOPE_COLON_MISSING = "':' - expected";
const char* const Deserializer::ERROR_PARSER_ARRAY_SCOPE_OPEN = "'[' - expected";
const char* const Deserializer::ERROR_PARSER_ARRAY_SCOPE_CLOSE = "']' - expected";
  
void Deserializer::skipScope(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret, v_char8 charOpen, v_char8 charClose){
  
  p_char8 data = caret->getData();
  v_int32 size = caret->getSize();
  v_int32 pos = caret->getPosition();
  v_int32 scopeCounter = 0;
  
  bool isInString = false;
  
  while(pos < size){
    v_char8 a = data[pos];
    if(a == charOpen){
      if(!isInString){
        scopeCounter ++;
      }
    } else if(a == charClose){
      if(!isInString){
        scopeCounter --;
        if(scopeCounter == 0){
          caret->setPosition(pos + 1);
          return;
        }
      }
    } else if(a == '"') {
      isInString = !isInString;
    } else if(a == '\\'){
      pos ++;
    }
    
    pos ++;
    
  }
}
  
void Deserializer::skipString(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  p_char8 data = caret->getData();
  v_int32 size = caret->getSize();
  v_int32 pos = caret->getPosition();
  v_int32 scopeCounter = 0;
  while(pos < size){
    v_char8 a = data[pos];
    if(a == '"'){
      scopeCounter ++;
      if(scopeCounter == 2) {
        caret->setPosition(pos + 1);
        return;
      }
    } else if(a == '\\'){
      pos ++;
    }
    pos ++;
  }
}
  
void Deserializer::skipToken(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  p_char8 data = caret->getData();
  v_int32 size = caret->getSize();
  v_int32 pos = caret->getPosition();
  while(pos < size){
    v_char8 a = data[pos];
    if(a == ' ' || a == '\t' || a == '\n' || a == '\r' || a == '\b' || a == '\f' ||
       a == '}' || a == ',' || a == ']') {
      caret->setPosition(pos);
      return;
    }
    pos ++;
  }
}
  
void Deserializer::skipValue(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->isAtChar('{')){
    skipScope(caret, '{', '}');
  } else if(caret->isAtChar('[')){
    skipScope(caret, '[', ']');
  } else if(caret->isAtChar('"')){
    skipString(caret);
  } else {
    skipToken(caret);
  }
}
  
Deserializer::AbstractPtrWrapper Deserializer::readStringValue(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(String::Class::getType());
  } else {
    return AbstractPtrWrapper(oatpp::parser::json::Utils::parseString(caret), String::Class::getType());
  }
}

Deserializer::AbstractPtrWrapper Deserializer::readInt32Value(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(Int32::PtrWrapper::Class::getType());
  } else {
    return AbstractPtrWrapper(Int32::createAbstract(caret->parseInt32()), Int32::PtrWrapper::Class::getType());
  }
}

Deserializer::AbstractPtrWrapper Deserializer::readInt64Value(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(Int64::PtrWrapper::Class::getType());
  } else {
    return AbstractPtrWrapper(Int64::createAbstract(caret->parseInt64()), Int64::PtrWrapper::Class::getType());
  }
}

Deserializer::AbstractPtrWrapper Deserializer::readFloat32Value(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(Float32::PtrWrapper::Class::getType());
  } else {
    return AbstractPtrWrapper(Float32::createAbstract(caret->parseFloat32()), Float32::PtrWrapper::Class::getType());
  }
}

Deserializer::AbstractPtrWrapper Deserializer::readFloat64Value(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(Float64::PtrWrapper::Class::getType());
  } else {
    return AbstractPtrWrapper(Float64::createAbstract(caret->parseFloat64()), Float64::PtrWrapper::Class::getType());
  }
}

Deserializer::AbstractPtrWrapper Deserializer::readBooleanValue(const std::shared_ptr<oatpp::parser::ParsingCaret>& caret){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper(Boolean::PtrWrapper::Class::getType());
  } else {
    return AbstractPtrWrapper(Boolean::createAbstract(caret->parseBoolean("true", "false")), Boolean::PtrWrapper::Class::getType());
  }
}
  
Deserializer::AbstractPtrWrapper Deserializer::readObjectValue(const Type* const type,
                                                        const std::shared_ptr<oatpp::parser::ParsingCaret>& caret,
                                                        const std::shared_ptr<Config>& config){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper::empty();
  } else {
    return readObject(type, caret, config);
  }
}
  
Deserializer::AbstractPtrWrapper Deserializer::readListValue(const Type* const type,
                                                      const std::shared_ptr<oatpp::parser::ParsingCaret>& caret,
                                                      const std::shared_ptr<Config>& config){
  if(caret->proceedIfFollowsText("null")){
    return AbstractPtrWrapper::empty();
  } else {
    return readList(type, caret, config);
  }
}
  
Deserializer::AbstractPtrWrapper Deserializer::readValue(const Type* const type,
                                                  const std::shared_ptr<oatpp::parser::ParsingCaret>& caret,
                                                  const std::shared_ptr<Config>& config){
  
  auto typeName = type->name;
  if(typeName == oatpp::data::mapping::type::__class::String::CLASS_NAME){
    return readStringValue(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::Int32::CLASS_NAME){
    return readInt32Value(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::Int64::CLASS_NAME){
    return readInt64Value(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::Float32::CLASS_NAME){
    return readFloat32Value(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::Float64::CLASS_NAME){
    return readFloat64Value(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::Boolean::CLASS_NAME){
    return readBooleanValue(caret);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractObject::CLASS_NAME){
    return readObjectValue(type, caret, config);
  } else if(typeName == oatpp::data::mapping::type::__class::AbstractList::CLASS_NAME){
    return readListValue(type, caret, config);
  } else {
    skipValue(caret);
  }
  
  return AbstractPtrWrapper::empty();
  
}
  
Deserializer::AbstractPtrWrapper Deserializer::readList(const Type* type,
                                                 const std::shared_ptr<oatpp::parser::ParsingCaret>& caret,
                                                 const std::shared_ptr<Config>& config){
  
  if(caret->canContinueAtChar('[', 1)) {
    
    auto listWrapper = type->creator();
    oatpp::data::mapping::type::PolymorphicWrapper<AbstractList>
    list(std::static_pointer_cast<AbstractList>(listWrapper.getPtr()), listWrapper.valueType);
    
    Type* itemType = *type->params.begin();
    
    while(!caret->isAtChar(']') && caret->canContinue()){
      
      caret->findNotBlankChar();
      auto item = readValue(itemType, caret, config);
      if(caret->hasError()){
        return AbstractPtrWrapper::empty();
      }
      
      list->addPolymorphicItem(item);
      caret->findNotBlankChar();
      
      caret->canContinueAtChar(',', 1);
      
    }
    
    if(!caret->canContinueAtChar(']', 1)){
      if(!caret->hasError()){
        caret->setError(ERROR_PARSER_ARRAY_SCOPE_CLOSE);
      }
      return AbstractPtrWrapper::empty();
    };
    
    return AbstractPtrWrapper(list.getPtr(), list.valueType);
  } else {
    caret->setError(ERROR_PARSER_ARRAY_SCOPE_OPEN);
    return AbstractPtrWrapper::empty();
  }
  
}
  
Deserializer::AbstractPtrWrapper Deserializer::readObject(const Type* type,
                                                   const std::shared_ptr<oatpp::parser::ParsingCaret>& caret,
                                                   const std::shared_ptr<Config>& config){
  
  if(caret->canContinueAtChar('{', 1)) {
    
    auto object = type->creator();
    auto fieldsMap = type->properties;
    
    while (!caret->isAtChar('}') && caret->canContinue()) {
      
      caret->findNotBlankChar();
      auto key = Utils::parseStringToStdString(caret);
      if(caret->hasError()){
        return AbstractPtrWrapper::empty();
      }
      
      auto fieldIterator = fieldsMap->find(key);
      if(fieldIterator != fieldsMap->end()){
        
        caret->findNotBlankChar();
        if(!caret->canContinueAtChar(':', 1)){
          caret->setError(ERROR_PARSER_OBJECT_SCOPE_COLON_MISSING);
          return AbstractPtrWrapper::empty();
        }
        
        caret->findNotBlankChar();
        
        auto field = fieldIterator->second;
        field->set(object.get(), readValue(field->type, caret, config));
        
      } else if (config->allowUnknownFields) {
        caret->findNotBlankChar();
        if(!caret->canContinueAtChar(':', 1)){
          caret->setError(ERROR_PARSER_OBJECT_SCOPE_COLON_MISSING);
          return AbstractPtrWrapper::empty();
        }
        caret->findNotBlankChar();
        skipValue(caret);
      } else {
        caret->setError(ERROR_PARSER_OBJECT_SCOPE_UNKNOWN_FIELD);
        return AbstractPtrWrapper::empty();
      }
      
      caret->findNotBlankChar();
      caret->canContinueAtChar(',', 1);
      
    };
    
    if(!caret->canContinueAtChar('}', 1)){
      if(!caret->hasError()){
        caret->setError(ERROR_PARSER_OBJECT_SCOPE_CLOSE);
      }
      return AbstractPtrWrapper::empty();
    }
    
    return object;
    
  } else {
    caret->setError(ERROR_PARSER_OBJECT_SCOPE_OPEN);
  }
  
  return AbstractPtrWrapper::empty();
  
}
  
}}}}
