#include <iostream>
#include <string>

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/AttributeDefinition.h>
#include <aws/dynamodb/model/GetItemRequest.h>

int main(int argc, char** argv) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  std::cout << "Greeter received: ";

  return 0;
}