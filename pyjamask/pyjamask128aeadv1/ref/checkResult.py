import sys

def checkEqual(file1,file2):
    file1 = file1.read()
    file2 = file2.read()
    for i in range(len(file1)):
        if file1[i] != file2[i]:
            return "Not equal"
    return("Equal")

if __name__ == '__main__':
    try:
        filename1 = sys.argv[1]
        filename2 = sys.argv[2]
    except:
        print("Please give two filenames to check")
    
    file1 = open(filename1,'r')
    file2 = open(filename2,'r')

    print(checkEqual(file1,file2))