# SimpleSock Socket Library

This is a simple library for using sockets across multiple platforms
(at least Windows and macOS).

# Git Submodule Tutorial

To add this project as a submodule to a git repository, run the
following command:

```bash
git submodule add https://github.com/sabarrett/simplesock.git
git commit -m "Add simplesock submodule
```

Note that, if you clone the newly-created repository (that is, the
repository that you added the submodule to), the submodule folder will
be empty! To correct this, run:

`git submodule update --init --recursive`

If SimpleSock updates and you want to acquire the updates to your
submodule, do the following from your repository root:

```bash
cd simplesock
git pull origin main
cd ..
git commit -am "Update simplesock submodule"
```
