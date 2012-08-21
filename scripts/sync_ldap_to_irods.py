from datetime import *
import sys, re 
import subprocess as sp

class SyncRunner(object):
    def __init__(self):
        self.users = []
        self.posixtime = self.get_posixtime()
        # Info types to sync, from LDAP
        self.infotypes = ["shadowExpire", "uid"]
        self.skipusers_patterns = ["grid"] 
        
    def run(self):
        self.posixtime = self.get_posixtime()
        self.add_new_users()
        # self.delete_expired_users()
        # self.connect_users_and_groups()
    
    def add_new_users(self):
        # TODO: Activate real method
        #userinfo = self.get_userinfo_from_ldap(self.infotypes)
        userinfo = self.get_userinfo_from_ldap_dummy(self.infotypes)
        
        for userpart in userinfo.split("\n\n"):
            try: 
                user = User()
                user.username = self.get_match("uid: (\w+)", 1, userpart)
                expiretime_d = int(self.get_match("shadowExpire: (\d+)", 1, userpart))
                user.expirytime = expiretime_d * 24 * 3600

                # Test if username does not contain any of the skip pattern 
                skip_user = False
                for pattern in self.skipusers_patterns:
                    if pattern in user.username:
                        skip_user = True
                
                if not skip_user:
                    self.users.append(user)
            except StopIteration:
                pass

        # TODO: Remove debug code
        for user in self.users:
            print user.username + ", " + str(user.expirytime)
        
    def get_userinfo_from_ldap(self, infotypes):
        ldapcmd = "ldapsearch -x -LLL '(uid=*)'"
        for infotype in infotypes:
            ldapcmd += " %s" % infotype
        output = self.exec_cmd(ldapcmd)
        return output

    def get_userinfo_from_ldap_dummy(self, infotypes):
        f = open("ldap_testdata.txt")
        output = f.read()
        return output
    
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

    def get_match(self, pattern, group, userpart):
        it = re.finditer(pattern, userpart, re.MULTILINE | re.DOTALL)
        m = it.next()
        match = m.group(group)
        return match

    
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
    
class User(object):
    def __init__(self):
        self.username = None
        self.expirytime = None
        
    
if __name__ == "__main__":
    syncrunner = SyncRunner()
    syncrunner.run()