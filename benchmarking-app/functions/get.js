import * as dynamoDbLib from "../libs/dynamodb-lib";
import { success, failure } from "../libs/response-lib";

export async function main(event, context) {
    const params = {
        TableName: process.env.tableName,
        Key: {
            benchmarkId: event.pathParameters.id
        }
    };
    try {
        const result = await dynamoDbLib.call("get", params);
        // Return the matching list of items in response body
        return success(result);
      } catch (e) {
        return failure({ status: false });
    }
}