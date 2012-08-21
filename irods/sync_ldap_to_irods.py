from datetime import *
import sys, os, commands 
import subprocess as sp

class SyncRunner(object):
    def run(self):
        posixtime = self.get_posixtime()
        print posixtime
        
        self.add_new_users()
        # self.delete_expired_users()
        # self.connect_users_and_groups()
    
    def add_new_users(self):
        infotypes = ["shadowExpire", "uid"]
        userinfo = self.get_userinfo_from_ldap(infotypes)
        
    def get_userinfo_from_ldap(self, infotypes):
        ldapcmd = "ldapsearch -x -LLL '(uid=*)'"
        for infotype in infotypes:
            ldapcmd += " %s" % infotype
        output = self.exec_cmd(ldapcmd)
    
    def get_posixtime(self):
        now = datetime.now()
        datestr = now.strftime("%s")
        return datestr
    
    def exec_cmd(self, command):
        output = ""
        try:
            output = sp.Popen(command.split(" "), stdout=sp.PIPE)
        except Exception:
            sys.stderr.write("ERROR: Could not execute command: " + command)
            raise
        return output
    
#    def get_users_from_ldap(self):
#        #cmd = 
#        # Get output
#        "ldapsearch -x -LLL '(uid=*)' shadowExpire uid" +
#        # Remove lines containing "dn:"
#        "grep -v dn:|" +
#        # Remove labels "blabla:"
#        "sed -r 's/^.*://g'|" +
#        # Replace newlines with tabs 
#        "tr "\n" "\t"|\" + 
#        # Replace double tabs with newlines
#        "sed 's/\t\t/\n/g'|" +
#        # Delete spaces 
#        "tr -d " "|" + 
#        # Remove grid-users
#        "grep -v -P \"\tgrid\"|\" +
#        # Convert from posix time in days to posix time in seconds
#        # and compare to current time.
#        "awk '$1*3600*24 >= '$now' { print $2 }'|" +
#        # Delete empty rows
#        "sed '/^$/d'"
    
if __name__ == "__main__":
    syncrunner = SyncRunner()
    syncrunner.run()