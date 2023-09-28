# BrainTell Backend Service Explanation
1. The service adopts a layered architecture:

   1. The service layer (service) is directly called by HTTP, responsible for transforming user request parameters and returning responses.
   2. The aggregation layer (ao) aggregates data returned from multiple data layers (do).
   3. The data layer (do) is directly responsible for database-related operations.
   4. Models (models) represent the database protocols.
   5. Utils (utils) includes some commonly used interfaces, such as encoding/decoding, image cropping, and SWC file cropping functions.
   6. Third-party (thirdparty) contains source code for third-party modules:
      1. CollServer is the real-time collaboration service code.
      2. cropswc is the source code for the SWC file cropping program.
   7. This project is developed using the Go programming language.

2. Familiarize yourself with the following tools used in the project:

   1. Go language
   2. XORM
   3. Redis
   4. HTTP

3. The project uses a configuration file (config) to manage settings:

   1. MySQL configuration
   2. Redis configuration
   3. aeskey: AES encryption string (not used)
   4. mainpath: main folder path
   5. cropprocess: concurrency level for cropswc and cropimage
   6. emails: recipients for daily reports

4. The project is containerized using Docker:

   1. Nginx container
   2. MySQL container
   3. Redis container
   4. Web service container