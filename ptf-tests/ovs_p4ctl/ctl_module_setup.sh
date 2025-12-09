cd ovsp4ctl/ovsp4ctl/
cp ../../../utilities/ovs-p4ctl.in .
mv ovs-p4ctl.in ovs_p4ctl.py
cd ..
python setup.py sdist
pip install ./dist/ovsp4ctl-0.0.1.tar.gz
