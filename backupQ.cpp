#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>

#include <sys/stat.h>
#include <sqlite3.h> 

// Test function to see how the callback works, just displays results of query
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

// Called if the first parameter is 1, the second parameter is included in the query
// Returns 1 if the value specified in the query is found, followed by the name.
// Returns 0 if it's not found
// Confirms the name given in the second parameter exists
// OR
// Called if the first parameter is 3, the second parameter isn't used.
// Returns 3 and the name recorded against the oldest backup date. 
static int if_exists(void *data, int argc, char **argv, char **azColName){
   int i;
   bool found(false);
   for(i = 0; i < argc; i++){
	found = true;
	std::cout << (const char*)data << argv[i] << std::endl;
   }
   if(!found) {
	   std::cout << "0" << std::endl;
   }
   return 0;
}

// Reads the location of the database file
std::string read_config()
{
	// Make this also read the suggested prefix for the label file names
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

// Called if the second parameter is 2 and doesn't use the second parameter.
// Returns 2 and the name of the new item if it works.
// Returns 0 if it doesn't work.
// It sorts the names in the database in descending order, finds the highest number and increments it by 1 to generate the next name.
void add_media(sqlite3 *db)
{
	bool worked = false;
	const std::string file_name_prefix = "music_server_backup-"; // Should I read this from the config file?
	const std::string file_name_ext = ".dsk";	
	std::string file_name;
	std::string statement;
	sqlite3_stmt *stmt = NULL;
        const char *sql_get_latest = "SELECT Media.Name FROM Media ORDER BY Media.Name DESC LIMIT 1;";	// Find the highest numbered media
	if(!sqlite3_prepare_v2(db, sql_get_latest, -1, &stmt, 0)) { // check it retrieved something/for errors
        	if(sqlite3_step(stmt) == SQLITE_ROW) { // If there is a name
            		file_name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
			int number = std::stoi(file_name.substr(file_name_prefix.size(), 3));
			file_name = file_name_prefix;
			std::string tmp = std::to_string(++number);
			while(tmp.size() < 3) {
				tmp.insert(0, 1, '0');
			}
			file_name += tmp;
			file_name += file_name_ext;
		} else { // If there isn't a name create a default one.
			file_name = file_name_prefix;
			file_name += "001";
			file_name += file_name_ext;
		}
		sqlite3_finalize(stmt);
		int key = 0;
		sqlite3_stmt *state = NULL;
		statement = "INSERT INTO Media (Name) VALUES ('";
		statement.append(file_name);
		statement += "');";
		if(!sqlite3_prepare_v2(db, statement.c_str(), -1, &state, 0)) {
			if(sqlite3_step(stmt) == SQLITE_DONE) {
				sqlite3_finalize(stmt);
				sqlite3_stmt *state = NULL;
				statement = "SELECT Media_Key FROM Media WHERE (Name = '";
				statement.append(file_name);
				statement += "');";
				if(!sqlite3_prepare_v2(db, statement.c_str(), -1, &state, 0)) { // check it retrieved something/for errors
        				if(sqlite3_step(state) == SQLITE_ROW) { // If there is a name, get its key
            					key = (int)sqlite3_column_int(state, 0);
						const char *update = "INSERT INTO History (F_Media_Key, Backup_Date) VALUES (?, ?);";
						sqlite3_stmt *stmt;
						if(sqlite3_prepare(db, update, -1, &stmt, NULL ) != SQLITE_OK) {
							std::cerr << "Statement not correctly prepared!" << std::endl;						}
						time_t     now = time(0);
						struct tm  tstruct;
						char       buf[80];
						tstruct = *localtime(&now);
						strftime(buf, sizeof(buf), "2000-%m-%d", &tstruct);
						if(sqlite3_bind_int(stmt, 1, key) != SQLITE_OK) {
							std::cerr << "Could not add key value: " << key << std::endl;
						}
						if(sqlite3_bind_text(stmt, 2, buf, 11, SQLITE_STATIC) != SQLITE_OK) {
							std::cerr << "Could not add date value: " << buf << std::endl; 
						}
						if(sqlite3_step(stmt) == SQLITE_DONE) { 
							std::cout << "2-" << file_name << std::endl;
							worked = true;
						} 
						sqlite3_finalize(stmt);
 					}
				}
				sqlite3_finalize(state);
			}
		}
	}
	if(!worked) {
		std::cout << "0" << std::endl;
	}
}

void update_record(sqlite3 *db, const std::string name)
{
	const char *sql = "UPDATE History SET Backup_Date = ? WHERE  F_Media_Key = (SELECT Media.Media_key FROM Media WHERE (Media.Name = ?));";
	sqlite3_stmt *stmt;
	if(sqlite3_prepare(db, sql, -1, &stmt, NULL ) != SQLITE_OK) {
		std::cerr << "Statement not correctly prepared!" << std::endl;					       }
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
	if(sqlite3_bind_text(stmt, 1, buf, 11, SQLITE_STATIC) != SQLITE_OK) {
		std::cerr << "Could not add date value: " << buf << std::endl;
	}
	if(sqlite3_bind_text(stmt, 2, name.c_str(), name.size(), SQLITE_STATIC) != SQLITE_OK) {
		std::cerr << "Could not add date value: " << name.c_str() << std::endl; 
	}
	if(sqlite3_step(stmt) == SQLITE_DONE) { 
		std::cout << "4-" << name << std::endl;
	} 
	sqlite3_finalize(stmt);
}

int main(int argc, char* argv[]) {
// Check there are 2 parameters, that the first is a number and the second a string
// If the above is true, feed the number into a case statement, that passes the string to an appropriate callback function.
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
//      return 4 and the name if successful. 
//      UPDATE History SET Backup_Date = '20210703' WHERE  F_Media_Key = (SELECT Media.Media_key FROM Media WHERE (Media.Name = 'madmikes_backup_002.vol'));

//	write some PHP to display the list of disks in the database, ordered to show the oldest at the top (in red)
//	It should include the name and data of backup, but I might be able to do something like the time it took to carry out.
//	SELECT Media.Name, History.Backup_Date FROM Media INNER JOIN History ON Media.Media_key = History.F_Media_key ORDER BY History.Backup_Date ASC;
	std::string db_location = read_config();
	struct stat buffer;
	if((stat(db_location.c_str(), &buffer) == 0) && ((argc == 2) || (argc == 3))) {
		int question;
		std::string disk;
		std::istringstream ss(argv[1]);
		if (!(ss >> question)) {
			  std::cerr << "Invalid number: " << argv[1] << '\n';
			  return 0;
		} else if (!ss.eof()) {
			  std::cerr << "Trailing characters after number: " << argv[1] << '\n';
			  return 0;
		}
		if((question > 0) && (question < 5)) {
			if((question == 1) || (question == 4)) {
				if(argc == 3) {		
					disk = argv[2];
				} else {
					std::cerr << "Disk Name argument not provided!" << std::endl;
					return 0;
				}
			}
			sqlite3 *db;
			char *zErrMsg = 0;
			int rc;
			if(sqlite3_open(db_location.c_str(), &db)) {
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl; 
	    			return(0);
			}
			const char* disk_name = disk.c_str();
                        std::string statement;
   			switch(question) {
	        		case 1 :
					{
						statement = "SELECT Media.Name FROM Media WHERE (Media.Name = \'";
				        	statement.append(disk_name);
				        	statement.append("\');");
						const char *sql = statement.c_str();
					        const char *action = "1-";	
						rc = sqlite3_exec(db, sql, if_exists, (void*)action, &zErrMsg);
					}
					break;
				case 2 :
					{
						add_media(db);
					}
					break;
				case 3 :
					{
						const char *sql =  "SELECT Media.Name FROM Media INNER JOIN  History ON History.F_Media_Key = Media.Media_key ORDER BY History.Backup_Date ASC LIMIT 1;";
						const char *action = "3-";
						rc = sqlite3_exec(db, sql, if_exists, (void*)action, &zErrMsg);
						}
			        	break;
				case 4 :
			        	update_record(db, disk_name);
					break;
				default :
					 std::cerr << "Invalid Choice" << std::endl;
					 return(0);
			}
		
			if( rc != SQLITE_OK ) {
				std::cerr << "SQL error: " << zErrMsg << std::endl;
	    			sqlite3_free(zErrMsg);
			} 
			sqlite3_close(db);
		}	
	} else {
		std::cerr << "Wrong number of command parameters!" << std::endl;
	}
}

