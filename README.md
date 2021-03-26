# Parallel Lightweight Cryptography
An investigation into parallelising lightweight cryptographic algorithms submitted to the NIST LWC competition. Part of my final year Computer Science Project.

## Design
### Program
The program compiles and executes the algorithms, and then uploads the results for verification and comparison. It is controlled through the main.py Python file which handles all the compilation of the c files which themselves exectue the tests. The use of Python was to help the upload to AWS, C was used for the benchmarking as this made the benchmarking of system information, such as memory, easier to achieve.

### App
The application is an AWS serverless backend which is used to hold the results from running the program. AWS was chosen as the project, in theory, should be able to allow anyone to upload their results from their machine; however, enough power was required to perform the checking that the algorithms have not been tampered with. 

### Website
The simple website displays the results stored in the database. This was produced for my supervisors so they were able to see the results in more detail if they wanted to.

## Notes
If you would like to read the final paper, please contact me at www.linkedin.com/in/harry-brooks-comp
