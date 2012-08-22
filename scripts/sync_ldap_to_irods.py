#!/usr/bin/python

from datetime import *
import sys, re, os 
import subprocess as sp
from nose.tools import assert_equal
from nose.tools import assert_not_equal

class SyncRunner(object):
    def __init__(self):
        self.users = []
        # Info types to sync, from LDAP
        self.infotypes = ["shadowExpire", "uid"]
        self.skipusers_patterns = ["grid"] 
        
    def run(self):
        '''
        The main method of the script
        '''
        ldapdata = self.get_userinfo_from_ldap(self.infotypes)
        users = self.parse_ldap_data_to_users(ldapdata)
        users = self.filter_usernames(users)
        users = self.filter_expirytimes(users)
        self.users = users
        
        irods = IRodsConnector()
        
        for user in self.users:
            if not irods.user_exists(user.username):
                print "User %s missing, so creating now ..." % user.username
                irods.create_user(user.username, usertype="rodsuser")
                
        # self.delete_expired_users()
        # self.connect_users_and_groups()
    

    def parse_ldap_data_to_users(self, ldapdata):
        users = []
        for userpart in ldapdata.split("\n\n"):
            try: 
                user = User()
                user.username = self.get_match("uid: (\w+)", 1, userpart)
                expiretime_d = int(self.get_match("shadowExpire: (\d+)", 1, userpart))
                user.expirytime = expiretime_d * 24 * 3600
                users.append(user)
            except StopIteration:
                pass

        return users
        
    def filter_usernames(self, users):
        def contains_pattern(teststring, patterns):
            contains_pattern = False
            for pattern in patterns:
                if pattern in teststring:
                    contains_pattern = True
            return contains_pattern

        filtered_users = []
        for user in users:
            # Test if username does not contain any of the skip pattern 
            if not contains_pattern(user.username, self.skipusers_patterns):
                filtered_users.append(user)
        return filtered_users     

    def filter_expirytimes(self, users):
        filtered_users = []
        for user in users:
             if user.expirytime > user.get_posixtime():
                 filtered_users.append(user)
        return filtered_users
                       
    def get_userinfo_from_ldap(self, infotypes):
        ldapcmd = "ldapsearch -x -LLL '(uid=*)'"
        for infotype in infotypes:
            ldapcmd += " %s" % infotype
        output = exec_cmd(ldapcmd)
        return output

    def get_match(self, pattern, group, userpart):
        it = re.finditer(pattern, userpart, re.MULTILINE | re.DOTALL)
        m = it.next()
        match = m.group(group)
        return match

class User(object):
    def __init__(self):
        self.username = None
        self.expirytime = None

    def has_expired(self):
        has_expired = True
        self.now_posix = self.get_posixtime()
        if self.expirytime > self.now_posix: 
            has_expired = False
        return has_expired
    
    def get_posixtime(self):
        now = datetime.now()
        posixtime = int(now.strftime("%s"))
        return posixtime


class IRodsConnector(object):
    def __init__(self):
        self.icommands_path = "/opt/irods/iRODS/clients/icommands/bin"
        # Some paths to binaries:

    def create_user(self, username, usertype="rodsuser"):
        cmd = "%s mkuser %s %s" % (self.get_iadmin_p(), username, usertype)
        exec_cmd(cmd)
        
    def delete_user(self, username):
        cmd = "%s rmuser %s" % (self.get_iadmin_p(), username)
        exec_cmd(cmd)

    def user_exists(self, username):
        cmd = self.get_iadmin_p() + " lu " + username
        output = exec_cmd(cmd)
        output = str.strip(output)
        if output == "No rows found":
            return False
        else:
            return True
        
    def list_users_in_zone(self, zone):
        cmd = self.get_iadmin_p() + " luz " + zone
        output = exec_cmd(cmd)
        users = str.strip(output).split("\n")
        return users
        
        
    def get_iadmin_p(self):
        return os.path.join(self.icommands_path, "iadmin")
    

# Tests

class TestSyncRunner(object):
    @classmethod
    def setup_class(self):
        f = open("ldap_testdata.txt")
        self.testdata = f.read()
        f.close()
        self.syncrunner = SyncRunner()
        self.users = self.syncrunner.parse_ldap_data_to_users(self.testdata)
        self.expired_user = self.users[2]
        # Make one user already expired
        expired_time = self.expired_user.get_posixtime() - 3600
        self.expired_user.expirytime = expired_time 
        self.users = self.syncrunner.filter_usernames(self.users)
        self.users = self.syncrunner.filter_expirytimes(self.users)
        self.delete_all_users()

    @classmethod        
    def teardown_class(self):
        self.delete_all_users()

    def test_expirydate(self):
       for user in self.users:
            assert user.expirytime > user.get_posixtime()
            
    def test_username(self):
        for user in self.users:
            assert "grid" not in user.username

    def test_epirydate2(self):
        for user in self.users:
            assert_not_equal(self.expired_user.username, user.username)
            
    def test_blackbox(self):
        self.syncrunner.run()
        
    @classmethod    
    def delete_all_users(self):
        irods = IRodsConnector()
        for user in irods.list_users_in_zone("ssUppnexZone"):
            if not "rods" in user:
                sys.stderr.write("Now deleting user " + user + "...\n")
                irods.delete_user(user)



# Some global methods

def exec_cmd(command):
    output = ""        
    try:
        print("Now executing: " + command)
        commandlist = command.split(" ")
        p = sp.Popen(commandlist, stdout=sp.PIPE)
        output = p.stdout.read()
    except Exception:
        sys.stderr.write("ERROR: Could not execute command: " + command)
        raise
    return output

# Run main

if __name__ == "__main__":
    syncrunner = SyncRunner()
    syncrunner.run()