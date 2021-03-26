import { success, failure } from "../libs/response-lib";
import { hash } from "../libs/hash-lib";

export async function main(event, context) {
    const body = JSON.parse(event.body);
    const folderItems = body["folderItems"];
    const result = body.result;
    try {
        const resultHash = hash(folderItems, result);
        // Return the matching list of items in response body
        return success({resultHash});
      } catch (e) {
        return failure({ status: false });
    }
}