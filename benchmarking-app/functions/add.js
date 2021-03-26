import uuid from "uuid";
import * as dynamoDbLib from "../libs/dynamodb-lib";
import { success, failure } from "../libs/response-lib";
import AWS from 'aws-sdk';
import { hash } from '../libs/hash-lib';

const s3 = new AWS.S3();
const bucket = "evaluation-algorithms";

export async function main(event, context) {
  try{
    const body = JSON.parse(event.body);
    const data = await s3.listObjects({Bucket: bucket, Prefix: body.result.algorithm_name}).promise();
    let files = [];
    for(var file of data.Contents) {
      const value = await s3.getObject({Bucket: bucket, Key: file.Key}).promise();
      files.push(value.Body.toString());
    }
    const resultHash = hash(files, body.result);
    if(resultHash === body.resultHash) {
      const params = {
        TableName: process.env.tableName,
        Item: {
          benchmarkId: uuid.v1(),
          ...body.result
        }
      };
      console.log(params);
      try {
        await dynamoDbLib.call("put", params);
        return success(params.Item);
      } catch (e) {
        console.error(e);
        return failure({ status: false });
      }
    }
    else {
      console.error("Hashes do not match");
    }
  }
  catch(e) {
    console.error(e);
  }
}