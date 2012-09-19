#!/usr/bin/python

from datetime import *
import sys, re, os 
import subprocess as sp
#from nose.tools import assert_equal
#from nose.tools import assert_not_equal

#getent_path = "/home/samuel/wksp/irods/scripts/getent"
getent_path = "/usr/bin/getent"

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

        # Parse LDAP data to user objects
        #ldapdata = self.get_userinfo_from_ldap(self.infotypes)
        #users = self.parse_ldap_data_to_users(ldapdata)
        #users = self.filter_usernames(users)
        #users = self.filter_expired_users(users)
        #self.users = users

        # Get all groups
        groups = self.get_groups()
        # print("Groups:\n%s" % groups)
        groups = self.filter_groups(groups)
        projects_on_swestore = self.get_projects_from_swestore()
        users = []
        usernames_all = []

        for proj in projects_on_swestore:
            usernames = groups[proj].usernames
            usernames_all.extend(usernames)

        usernames_all = list(set(usernames_all)) # Sort the list

        for username in usernames_all:
            user = User()
            user.username = username
            self.users.append(user)

        # Get an iRODS connector to talk to iRODS
        irods = IRodsConnector()
        
        # Create users
        for user in self.users:
            if not irods.user_exists(user.username):
                print("User %s missing, so creating now ...\n" % user.username)
                irods.create_user(user.username, usertype="rodsuser")
               

        # Create groups
        for groupname, group in groups.items():
            if not irods.group_exists(group.groupname):
                print("Creating group %s ..." % group.groupname)
                irods.create_group(group.groupname)

        # Connect users and groups
        for groupname, group in groups.items():
            groupusers_in_irods = irods.list_group_users(group.groupname)
            for username in group.usernames:
                if username in groupusers_in_irods:
                    print("User %s already connected to group %s" % (username, group.groupname))
                    pass
                elif irods.user_exists(username):
                    print("Adding user %s to group %s ..." % (username,group.groupname))
                    irods.add_user_to_group(username, group.groupname)
                    
        # Create project folders for groups
        projfolder = "/swestore-legacy/proj"
        if not irods.folder_exists(projfolder):
            irods.create_folder(projfolder)
            pass
        for group in groups:
            groupfolder = os.path.join(projfolder, group)
            if not irods.folder_exists(groupfolder):
                print("Creating folder %s ..." % groupfolder)
                irods.create_folder(groupfolder)
                irods.make_owner_of_folder(group, groupfolder)
                irods.set_inherit_on_folder(groupfolder)
                irods.remove_access_to_folder("public", groupfolder)

        # iReg files

        ss_path = "srm://srm.swegrid.se/snic/uppnex/arch/proj"
        for proj in projects_on_swestore:
            ss_projpath = ss_path + "/" + proj
            irods_projpath = "/swestore-legacy/proj/" + proj

            cmd = "arcls %s" % (ss_projpath)
            output = exec_cmd(cmd)
            arch_mssns = output.strip().split("\n")
            for am in arch_mssns:
                ss_ampath = ss_projpath + "/" + am
                irods_ampath = irods_projpath + "/" + am
                if not irods.folder_exists(irods_ampath):
                    print("Folder %s did not exist, so creating ..." % irods_projpath)
                    irods.create_folder(irods_ampath)
                    irods.set_inherit_for_folder(irods_ampath)
                cmd = "arcls %s" % (ss_ampath)
                output = exec_cmd(cmd)
                files = output.strip().split("\n")
                for f in files:
                    # ireg -R swestoreArchResc -G swestoreArchGrp /proj/$p/$d/$f /swestore-legacy/proj/$p/$d/$f;
                    filepath = "/proj/%s/%s/%s" % (proj, am, f)
                    irods_filepath = "/swestore-legacy" + filepath
                    irods.ireg_file(filepath, irods_filepath, "swestoreLegacyArchResc", "swestoreLegacyArchGrp")
                    print("Iregged file " + irods_filepath + " ...")


    def parse_ldap_data_to_users(self, ldapdata):
        '''
        Parse the output of the ldapsearch command to user objects
        ''' 
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

    def filter_expired_users(self, users):
        filtered_users = []
        for user in users:
             if user.expirytime > user.get_posixtime():
                 filtered_users.append(user)
        return filtered_users

    def get_userinfo_from_ldap(self, infotypes):
        ldapcmd = "ldapsearch -x -LLL '(uid=*)'"
        for infotype in infotypes:
            ldapcmd += " " + infotype
        output = exec_cmd(ldapcmd)
        return output

    def get_groups(self):
        groups = {}
        cmd = getent_path + " group"
        groupdata = exec_cmd(cmd)
        groupdata = groupdata.strip()
        for line in groupdata.split("\n"):
            cols = line.split(":")
            group = Group()
            group.groupname = cols[0]
            new_usernames = cols[3].split(",")
            for new_username in new_usernames:
                if new_username:
                    group.usernames.append(new_username)
            groups[group.groupname] = group
        return groups

        # |grep -P "^(a|b)20"|cut -f1 -d:)"
        
    def filter_groups(self, groups):
        filtered_groups = {}
        for groupname,group in groups.items():
            if re.match("^(a|b|p|s)[0-9]{5}.*", groupname):
                filtered_groups[groupname] = group
            else:
                print("Group does not match pattern, so skipping: %s" % group.groupname)
        return filtered_groups

    def get_match(self, pattern, group, userpart):
        it = re.finditer(pattern, userpart, re.MULTILINE|re.DOTALL)
        m = it.next()
        match = m.group(group)
        return match

    def get_projects_from_swestore(self):
        swestore_proj_data = exec_cmd("arcls srm://srm.swegrid.se/snic/uppnex/arch/proj")
        projects = swestore_proj_data.strip().split("\n")
        return projects

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

class Group(object):
    def __init__(self):
        self.groupname = ""
        self.usernames = []

class IRodsConnector(object):
    def __init__(self):
        self.icommands_path = "/opt/irods/iRODS/clients/icommands/bin"
        self.usernames = self.list_users_in_zone("swestore-legacy")
        self.groupnames = self.list_groups()

    def user_exists(self, username):
        return username in self.usernames

    def create_user(self, username, usertype="rodsuser"):
        cmd = "%s mkuser %s %s" % (self.get_iadmin_p(), username, usertype)
        exec_cmd(cmd)
        
    def delete_user(self, username):
        cmd = "%s rmuser %s" % (self.get_iadmin_p(), username)
        exec_cmd(cmd)
        
    def list_users_in_zone(self, zone):
        cmd = self.get_iadmin_p() + " luz " + zone
        output = exec_cmd(cmd)
        users = str.strip(output).split("\n")
        return users
        
    def list_groups(self):
        cmd = self.get_iadmin_p() + " lg"
        groups = exec_cmd(cmd).strip().split("\n")
        return groups
    
    def list_group_users(self, group):
        cmd = self.get_iadmin_p() + " lg " + group
        users = exec_cmd(cmd).strip().split("\n")
        for k,v in enumerate(users):
            users[k] = v.replace("#swestore-legacy", "")
        return users
        
    def group_exists(self, groupname):
        return groupname in self.groupnames
        
    def create_group(self, groupname):
        cmd = self.get_iadmin_p() + " mkgroup " + groupname
        exec_cmd(cmd)
        
    def delete_group(self, groupname):
        cmd = self.get_iadmin_p() + " rmgroup " + groupname
        exec_cmd(cmd)
        
    def group_has_user(self, group, user):
        cmd = self.get_iadmin_p() + " lg " + group
        output = exec_cmd(cmd)
        if user + "#" in output:
            return True
        else:
            return False
            
    def add_user_to_group(self, username, groupname):
        if username and groupname:
            cmd = self.get_iadmin_p() + " atg %s %s" % (groupname, username)
            exec_cmd(cmd)
        else:
            print("Username %s or group %s is empty!" % (username, groupname))
        
    def folder_exists(self, folder):
        cmd = self.get_ils_p() + " " + folder
        stdout,stderr = exec_cmd(cmd, get_stderr=True)
        if "ERROR" in stderr:
            return False
        else:
            return True
        
    def create_folder(self, folder):
        cmd = self.get_imkdir_p() + " " + folder
        exec_cmd(cmd)
        
    def delete_folder(self, folder, recursive=False):
        if recursive:
            cmd = self.get_irm_p() + " -rf " + folder
        else:
            cmd = self.get_irm_p() + folder
        exec_cmd(cmd)
        
    def make_owner_of_folder(self, owner, folder):
        cmd = "%s own %s %s" % (self.get_ichmod_p(), owner, folder)
        exec_cmd(cmd)
        
    def remove_access_to_folder(self, userorgroup, folder):
        cmd = "%s null %s %s" % (self.get_ichmod_p(), userorgroup, folder)
        exec_cmd(cmd)
        
    def set_inherit_on_folder(self, folder):
        cmd = "%s inherit %s" % (self.get_ichmod_p(), folder)
        exec_cmd(cmd)

    def ireg_file(self, filepath, irods_filepath, resource, resource_group):
        cmd = "%s -R %s -G %s %s %s" % (self.get_ireg_p(), resource, resource_group, filepath, irods_filepath)
        exec_cmd(cmd)

    def get_iadmin_p(self):
        return os.path.join(self.icommands_path, "iadmin")
    
    def get_ils_p(self):
        return os.path.join(self.icommands_path, "ils")
    
    def get_imkdir_p(self):
        return os.path.join(self.icommands_path, "imkdir")
    
    def get_ichmod_p(self):
        return os.path.join(self.icommands_path, "ichmod")
    
    def get_irm_p(self):
        return os.path.join(self.icommands_path, "irm")

    def get_ireg_p(self):
        return os.path.join(self.icommands_path, "ireg")
    
    
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
        self.users = self.syncrunner.filter_expired_users(self.users)
        self.delete_all_users()
        self.delete_all_groups()
        self.delete_projfolder()

    @classmethod        
    def teardown_class(self):
        self.delete_all_users()
        self.delete_all_groups()
        self.delete_projfolder()

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
        for user in irods.list_users_in_zone("#swestore-legacy"):
            if not "rods" in user:
                #sys.stderr.write("Now deleting user " + user + "...\n")
                irods.delete_user(user)

    @classmethod    
    def delete_all_groups(self):
        irods = IRodsConnector()
        for group in self.syncrunner.get_groups():
            if not "public" in group.groupname and not "rodsadmin" in group.groupname and irods.group_exists(group.groupname):
                #sys.stderr.write("Now deleting group " + group.groupname + "...\n")
                try:
                    irods.delete_group(group.groupname)
                except:
                    sys.stderr.write("Could not delete group %s\n" % group.groupname)
                    
    @classmethod
    def delete_projfolder(self):
        folder = "/swestore-legacy/proj"
        irods = IRodsConnector()
        if irods.folder_exists(folder):
            irods.delete_folder(folder, recursive=True)


# Some global methods

def exec_cmd(command, get_stderr=False):
    #print("Now executing: " + command)
    commandlist = command.split(" ")
    stdout = ""  
    if get_stderr:
        stderr = ""
        try:
            p = sp.Popen(commandlist, stdout=sp.PIPE, stderr=sp.PIPE)
            stdout = p.stdout.read()
            stderr = p.stderr.read()
        except Exception:
            sys.stderr.write("ERROR: Could not execute command:\n%s\n" % command)
            raise
        return stdout,stderr
    else:
        try:
            p = sp.Popen(commandlist, stdout=sp.PIPE)
            stdout = p.stdout.read()
        except Exception:
            sys.stderr.write("ERROR: Could not execute command:\n%s\n" % command)
            raise
        return stdout

# Run main

if __name__ == "__main__":
    syncrunner = SyncRunner()
    syncrunner.run()
