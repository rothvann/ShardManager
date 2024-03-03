#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/AttributeDefinition.h>
#include <aws/dynamodb/model/GetItemRequest.h>
// snippet-end:[dynamodb.cpp.get_item.inc]
#include "dynamodb_samples.h"

int main(int argc, char** argv) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  Json::Reader reader;
  Json::Value obj;

  std::cout << "Greeter received: ";

  return 0;
}