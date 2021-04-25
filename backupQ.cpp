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

static int if_exists(void *data, int argc, char **argv, char **azColName){
   int i;
   bool found(false);
   for(i = 0; i<argc; i++){
	found = true;
  	std::cout << "1-" << argv[i] << std::endl;	
   }
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
// 	SELECT Media.Name FROM Media WHERE (Media.Name = 'madmikes_backup_002.vol');
// 2 - register the file
//      return 2 and the name if it worked, 0 if it didn't
//      the record date in the database should be in the year 2000, to ensure it picked up as the next suitable backup
//      SELECT Media.Name FROM Media ORDER BY Media.Name DESC LIMIT 1;
// 3 - is it the oldest backup (the best to overwrite)
//      return 3 and the name if it is
//      return 6 and the suggested name if it isn't
//      SELECT Media.Name, History.Backup_Date FROM Media INNER JOIN  History ON History.F_Media_Key = Media.Media_key ORDER BY History.Backup_Date ASC LIMIT 1;
// 4 - Update the database to return the name has just been backed up to.
//      return 5 and the name if succesful. 
//      UPDATE History SET Backup_Date = '20210703' WHERE  F_Media_Key = (SELECT Media.Media_key FROM Media WHERE (Media.Name = 'madmikes_backup_002.vol'));

//	write some PHP to display the list of disks in the database, ordered to show the oldest at the top (in red)
//	It should include the name and data of backup, but I might be able to do something like the time it took to carry out.
//	SELECT Media.Name, History.Backup_Date FROM Media INNER JOIN History ON Media.Media_key = History.F_Media_key ORDER BY History.Backup_Date ASC;
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
//		std::cout << "Prog: " << argv[0] << " count: " << argc << " 1: " << argv[1] << " 2: " << argv[2] << std::endl;
		if(((question > 0) && (question < 5)) && (disk.size() > 4)) {
			sqlite3 *db;
			char *zErrMsg = 0;
			int rc;
			const char* disk_name = disk.c_str();
//			std::cout << "Disk: " << disk_name << std::endl;
			rc = sqlite3_open(db_location.c_str(), &db);

			if( rc ) {
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl; 
	    			return(0);
			} else {
				std::cerr << "Opened database" << std::endl;
			}
		//	std::cout << "Pre Switch" << std::endl;	
			// Create SQL statement 
	//		const char *sql = "SELECT * from Media";
                        std::string statement;
   			switch(question) {
	        		case 1 :
					{
						statement = "SELECT Media.Name FROM Media WHERE (Media.Name = \'";
				        	statement.append(disk_name);
				        	statement.append("\');");
						const char *sql = statement.c_str(); 
						rc = sqlite3_exec(db, sql, if_exists, (void*)disk_name, &zErrMsg);
					}
					break;
				case 2 :
					{
						std::cout << "Register the file" << std::endl;
//						statement = 
//						rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
					}
					break;
				case 3 :
					{
			        		std::cout << "This is the oldest backup" << std::endl;
						const char *sql =  "SELECT Media.Name FROM Media INNER JOIN  History ON History.F_Media_Key = Media.Media_key ORDER BY History.Backup_Date ASC LIMIT 1;";
						rc = sqlite3_exec(db, sql, if_exists, (void*)disk_name, &zErrMsg);
						}
			        	break;
				case 4 :
					std::cout << "Update Database" << std::endl;
//					rc = sqlite3_exec(db, sql, callback, (void*)disk_name, &zErrMsg);
			        	break;
				default :
					 std::cout << "Invalid Question" << std::endl;
			}
//			std::cout << question << " - " << disk_name << std::endl;
		
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

