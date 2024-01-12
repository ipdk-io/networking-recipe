
The networking-recipe/stratum/security folder is being introduced as a
place-holder for credentials_manager and role_config work.

This is intended to be temporary until IPDK's Stratum fork downstreams
and syncs with the upstream Stratum repository.

### Credentials Manager
IPDK's Credentials Manager changes the default behavior of upstream Stratum by
making it secure-by-default. While this is still what we intend to keep, it
makes downstreaming code and sync'ing a nightmare. For this reason, we will
move the Credentials Manager under networking-recipe/stratum/security folder
keeping the secure-by-default behavior, while the rest of Stratum can be
downstreamed and updated.

### Role Config
Due to tight project deadlines (and to bypass delays in downstreaming Stratum to
IPDK fork) latest copy of Role Config support is pulled in to
networking-recipe/stratum/security folder.

Once downstreaming has been completed, build system CMake files can be updated
to use the original Stratum role config code in correct location.
