#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <sqlite3.h> 

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   printf("\n");
   for(i = 0; i<argc; i++){
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
		    
    printf("\n");
    return 0;
}

std::string read_config()
{
	std::string path;
	std::ifstream config_file;    
    	config_file.open("/etc/backupQ.conf");    
	if(config_file.is_open()){
		config_file >> path;
		config_file.close();
	} else {
		std::cerr << "/etc/backupQ.conf - not found!" << std::endl;
	}
	return path;
}

int main(int argc, char* argv[]) {
// Check there are 2 parameters, that the first is a number and the second a string
// if the above is true, feed the number into a case statement, that passes the string to an approiate callback function.
// the callback function should execute a query, use that to build a string and send that to stdout
//
// 1 - if the file exists in database
// 	return 1 and the name if does, 0 if doesn't
// 2 - register the file
//      return 2 and the name if it worked, 0 if it didn't
//      the record date in the database should be in the year 2000, to ensure it picked up as the next suitable backup
// 3 - is it the oldest backup (the best to overwrite)
//      return 3 and the name if it is
//      return 6 and the suggested name if it isn't
// 4 - Update the database to return the name has just been backed up to.
//      return 5 and the name if succesful. 

//	write some PHP to display the list of disks in the database, ordered to show the oldest at the top (in red)
//	It should include the name and data of backup, but I might be able to do something like the time it took to carry out.
	std::string db_location = read_config();
	if((argc == 3) && (db_location.size() > 0)) {
		int question;
		std::istringstream ss(argv[1]);
		if (!(ss >> question)) {
			  std::cerr << "Invalid number: " << argv[1] << '\n';
		} else if (!ss.eof()) {
			  std::cerr << "Trailing characters after number: " << argv[1] << '\n';
		}
		std::string disk = argv[2];
		if(((question > 0) && (question < 5)) && (disk.size() > 4)) {
			sqlite3 *db;
			char *zErrMsg = 0;
			int rc;
			const char* disk_name = disk.c_str();
	
			rc = sqlite3_open(db_location.c_str(), &db);

			if( rc ) {
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl; 
	    			return(0);
			} else {
				std::cerr << "Opened database" << std::endl;
			}
			
			// Create SQL statement 
			const char *sql = "SELECT * from updates";

   			switch(question) {
	        		case 1 :
					std::cout << "Does file exist in Database" << std::endl;
	      				// Execute SQL statement 
					rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
					break;
				case 2 :
					std::cout << "Register the file" << std::endl;
					rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
					break;
				case 3 :
			        	std::cout << "Is this the oldest backup" << std::endl;
					rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
			        	break;
				case 4 :
					std::cout << "Update Database" << std::endl;
					rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
			        	break;
				default :
					 std::cout << "Invalid Question" << std::endl;
			}
			std::cout << question << " - " << disk_name << std::endl;
		
			if( rc != SQLITE_OK ) {
				std::cerr << "SQL error: " << zErrMsg << std::endl;
	    			sqlite3_free(zErrMsg);
			} else {
				std::cerr << "Operation done sucessfully" << std::endl;
			}
			sqlite3_close(db);
		}	
	} else {
		std::cerr << "Wrong number of command parameters!" << std::endl;
	}
}

