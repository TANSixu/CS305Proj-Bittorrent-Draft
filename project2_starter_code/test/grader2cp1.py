#!/usr/bin/env python
import errno
import sys
import os
import time
from grader import grader, tester
import unittest2
import sys
import random
from subprocess import Popen, PIPE, STDOUT
import os.path
from plcommon import check_output, check_both
import resource
import subprocess
import re

BIN = "peer"
CHECKER = "python cp1_checker.py"

class project1cp1tester(tester):

    def __init__(self, test_name, testsuit):
        super(project1cp1tester, self).__init__(test_name, testsuit)

    def whohas_msg(self):
        print "Test to check if WHOHAS msg is sent correctly"
        if not os.path.isfile(BIN):
            print "%s is not found" % (BIN)
        print "Trying to start "
        cmd = "perl hupsim_1.pl -m %s -n %s -p %s -v %s" % \
              ("topo_1.map", "nodes.map", "15441", "0")
        print cmd
        pHumpsim = Popen(cmd.split(' '))
        print "Wait 1 seconds."
        time.sleep(1)
        print "Trying to start cp1_tester"
        cmd = "./cp1_tester -p %s -c %s -f %s -m 4 -i 1 -t 1 -d %s" % \
              ("nodes.map", "A.chunks", "C.chunks", "0")
        print cmd
        pTester = Popen(cmd.split(' '))
        print "Wait 1 seconds."
        time.sleep(1)
        print "Trying to start peer"
        cmd = "./peer -p %s -c %s -f %s -m 4 -i 2 -d %s" % \
              ("nodes.map", "B.chunks", "C.chunks", "0")
        print cmd
        pPeer = Popen(cmd.split(' '), stdout=PIPE, stdin=PIPE)
        print "Wait 1 seconds."        
        time.sleep(1)
        pPeer.stdin.write("GET A.chunks silly.tar\n")
        time.sleep(1)
        rc = pTester.wait()
        print "pTester rc = %d" % (rc)
        if rc == 10:
            print "success!"
            self.testsuite.scores['whohas_msg'] = 1
        else:
            print "fail!"
        pPeer.kill()
        pHumpsim.kill()

class project2cp1grader(grader):

    def __init__(self, checkpoint):
        super(project2cp1grader, self).__init__()
        self.process = None
        self.checkpoint = checkpoint
        os.environ["SPIFFY_ROUTER"] = "127.0.0.1:15441"
        print "SPIFFY_ROUTER is set to %s" % \
            (os.environ["SPIFFY_ROUTER"])
        killall = Popen(["sh","killall.sh"])
        rc = killall.wait()
        print "killall returns %d" % (rc)

    def prepareTestSuite(self):
        super(project2cp1grader, self).prepareTestSuite()
        self.suite.addTest(project1cp1tester('whohas_msg', self))
        self.scores['whohas_msg'] = 0

    def setUp(self):
        self.port = random.randint(1025, 9999)
        #self.port = 9999
        self.tls_port = random.randint(1025, 9999)
        self.tmp_dir = "../tmp/"
        self.priv_key = os.path.join(self.tmp_dir, 'grader.key')
        self.cert = os.path.join(self.tmp_dir, 'grader.crt')
        self.www = os.path.join(self.tmp_dir, 'www/')
        self.cgi = os.path.join(self.tmp_dir, 'cgi/cgi_script.py')
        print '\nUsing ports: %d,%d' % (self.port, self.tls_port)


if __name__ == '__main__':
    #resource.setrlimit(resource.RLIMIT_NOFILE, \
    #                  (2000, 2000))
    p1cp1grader = project2cp1grader("checkpoint-1")
    p1cp1grader.prepareTestSuite()
    p1cp1grader.setUp()
    results = p1cp1grader.runTests()
    p1cp1grader.reportScores()
