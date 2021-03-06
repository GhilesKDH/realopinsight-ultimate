RealOpInsight Ultimate is a web-based business service monitoring tool allowing advanced aggregation and correlation of monitoring events on top of existing systems, including Nagios®, Zabbix®, ManageEngine OpManager®, Pandora FMS®, Zenoss®, Icinga®, op5®, Centreon®, Shinken®, GroundWork®, and more.
RealOpInsight Workstation is released under the terms of GPLv3 License. See the LICENSE file.


RealOpInsight Workstation is released under the terms of GPLv3 License.
Prior to the installation, you may need to read and agree the license offer.
 

INSTALLATION
============
This component needs to be installed on the user workstation.

Requirements
============

For building and running RealOpInsight Ultimate, you will need the following:

- Ubuntu 14.04 LTS 64 bits
- GCC-C++ >= 4.7
- Qt version 5.5 with the following modules : QtCore, QtGui, QtSvg, QtWebKit, QtXml, QtScript. Other versions may work but are not tested.
- Graphviz
- Wt Web Toolkit version 3.3.4 or higher
- OpenLdap development files
 
Quick Installation
==================

You just have to follow the next steps, assuming you have a Ubuntu 14.04 LTS 64 bits with all the above dependencies installed.
*$* indicates a terminal prompt.

1. Checkout the latest source package from Github

  ```
  $ git clone https://github.com/RealOpInsightLabs/realopinsight-ultimate.git
  ```
2. Move to the source directory

  ```
  $ cd realopinsight-ultimate/
  ```

3. Launch the installation

  ```
  $ sudo ./install-manager -m ultimate
  ```
  Then wait that the installation completes.

4. Move forward
  Please go to RealOpInsight Ultimate documentation website at http://docs.realopinsight.com, 
  go throughout our tutorials and user/administrator manuals to start mastering the software. 

Security note about integration with PostgreSQL
==============================================
It's worth to note that the role in PostgreSQL does not need:

* to be a superuser
* to be allowed to create databases
* to be allowed to create more new roles



