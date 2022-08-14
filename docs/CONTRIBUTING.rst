=======================
Contributing to P4-OVS
=======================

As an open source project, we welcome contributions of any kind. These can
range from bug reports and code reviews, to significant code or documentation
features.

Source Code
-----------
IPDK's source code is hosted by `GitHub <https://github.com/ipdk-io/>`__ and patch submission and review are done through `Gerrit
<https://review.gerritipdk.com/gerrit>`__.

Contributing
------------

Patch submission is done through Gerrit where patches are voted on by everyone in the community.

A patch usually requires a minimum of two +2 votes before it will be merged. +2 votes are reserved for the **core maintainers** who can be contacted on the mailing list.

Gerrit Configuration
--------------------

Log into `Gerrit
<https://review.gerritipdk.com/gerrit>`__ using your GitHub account credentials. Once logged in, in the top right corner click your username and select ‘Settings’. You should set up the following:

- *Profile*: Verify that the information is correct here. This includes registering the e-mail address in your profile. The e-mail address must be registered before generating an HTTP password.
- *HTTP Password*: Generate a password. You’ll use this password when prompted by git (not your GitHub password!).

Gerrit repository locally
-------------------------

git clone and update submodule::

    $ git clone https://review.gerritipdk.com/gerrit/ipdk-io/ovs
    $ cd ovs
    $ git submodule update --init --recursive

In case you clone the repo already from GitHub, you can change your repository to point at Gerrit by doing::

    $ git remote set-url origin https://review.gerritipdk.com/gerrit/ipdk-io/ovs

When you later push a patch, you’ll be prompted for a password. That password is the one generated in the HTTP Password section of the Gerrit settings, not your GitHub password.

To make it easy, turn on the git credential helper to store your password for you. You can enable it for the IPDK repository with::

    $ git config credential.helper store

Example::

   $ cat .git-credentials
   https://<user-id>:8Ts3OsCL%2f23PlPQrZB8gobYJqtfoqCc4gKzQzuYzqTD@review.gerritipdk.com


Finally, you’ll need to install the Gerrit commit-msg hook. This inserts a unique change ID each time you commit and is required for Gerrit to work::

   $ cd "<repo>" && curl -Lo .git/hooks/commit-msg https://review.gerritipdk.com/gerrit/tools/hooks/commit-msg
   $ chmod +x .git/hooks/commit-msg

Now open .git/config in a text editor and add these lines to make reviews easier::

   [remote "review"]
   url = https://review.gerritipdk.com/gerrit/ipdk-io/ovs
   push = HEAD:refs/for/ovs-with-p4

Now you should be all set!

Submitting a Patch
------------------

All commits must be signed off by the developer which indicates that you agree to the `Developer Certificate of Origin <http://developercertificate.org/>`__ Developer Certificate of Origin. This is done using the -s or --signoff option when committing your changes.

Development on ipdk-io/ovs is  done based on the ovs-with-p4 branch, so start by making sure you have the latest. The below assumes origin is pointed at Gerrit::

   git checkout ovs-with-p4
   git pull

Next, create a branch for your development work::

   git checkout -b <my_branch>

Then, make your changes and commit as you go. You’ll build up a branch off of master with a series of commits. Once you are done, pull the latest from master again, rebase your changes on top, and update the submodule pointers that IPDK relies on::

   git checkout main
   git pull
   git checkout <my_branch>
   git rebase -i ovs-with-p4
   git submodule update

Now your branch should be based on the tip of ovs-with-p4 and you should have the tip of checked out. You can push your code to Gerrit for review by doing the following::

   git push review

If prompted for a password, remember that it is the password from the HTTP Password section of the Gerrit settings. If you enabled the git credential helper, you’ll only be prompted once

Continuous Integration
----------------------

The IPDK CI system periodically looks at Gerrit, pulls the patches down, and runs them on a pool of multiple machines.

When the CI system completes, it will post a comment on the Gerrit review with a +/-1 Verified flag, plus a link to the logs of the build run. Patches will not be merged without a +1 Verified from the CI system.

Code Review
-----------

Everyone is encouraged to review all patches and mark them with a +1 (thumbs up) or -1 (thumbs down). Code review feedback is highly valued, so even if you are a beginner with IPDK, please jump in and start reviewing patches. For a patch to be merged, two maintainers must give it a +2 vote, only maintainers are allowed to use +2.

Revising Patches
----------------

Gerrit makes it very easy to update an outstanding review. You simply update the commits in your git repository to incorporate the new changes and push again. For instance::

   git checkout <my_branch>
   <address code review feedback>
   git commit -a --amend
   git push review