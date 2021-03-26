import { sha256 } from "js-sha256";

export const hash = (folderItems, result) => {
    let message = "";
    for(let m of folderItems) {
        message += m;
    }
    message += result.system  + result.machine +
        result.algorithm_name  + result.additional_information + result.nist_O2 + result.nist_O0 + result.small_O2 +
        result.small_O0 + result.small_MP;

    message = message.replace(/\s/g,'');
    return sha256(message);
};