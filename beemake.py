import os
import shutil
import subprocess
import sys
import platform

def shell_call(command):
    subprocess.check_call(command, shell=True)

class Command(object):
    def __init__(self, name, help_short=None, *args, **kwargs):
        self.name = name
        self.help_short = help_short

    def execute(self, args, cwd):
        return 1

class genericcommand(Command):
    def __init__(self, *args, **kwargs):
        super(genericcommand, self).__init__(
            name='generic',
            help_short='Generic command',
            *args, **kwargs)

    def execute(self, args, cwd):
        print('BeeMake::Success!')
        return 0

class pullcommand(Command):
    def __init__(self, *args, **kwargs):
        super(pullcommand, self).__init__(
            name='pull',
            help_short='Pulls the repo and all dependencies.',
            *args, **kwargs)

    def execute(self, args, cwd):
        print('BeeMake::Pulling...')
        print('')

        print('BeeMake::Pulling self...')
        shell_call('git pull')
        print('')

        print('BeeMake::Pulling dependencies...')
        shell_call('git submodule update')
        print('')

        print('BeeMake::Success!')
        return 0

class nukecommand(Command):
    def __init__(self, *args, **kwargs):
        super(nukecommand, self).__init__(
            name='nuke',
            help_short='Removes all build/ output.',
            *args, **kwargs)

    def execute(self, args, cwd):
        print('BeeMake::Cleaning...')
        print('')
        print('BeeMake::Removing build/ ...')
        if os.path.isdir('build/'):
            shutil.rmtree('build/')
        print('')

        print('BeeMake::Success!')
        return 0

class buildcommand(Command):
    def __init__(self, *args, **kwargs):
        super(buildcommand, self).__init__(
            name='build',
            help_short='Builds the project',
            *args, **kwargs)

    def execute(self, args, cwd):
        print('BeeMake::Building...')
        os.chdir(os.getcwd())
        if not os.path.exists('build/'):
            os.makedirs('build/')
            print('')
        os.chdir('build/')

        print('BeeMake::Running CMake...')
        if platform.system() == 'Windows':
            shell_call('cmake .. -G "MSYS Makefiles"')
        else:
            shell_call('cmake .. -G "Unix Makefiles"')
        print('')

        print('BeeMake::Running make...')
        if platform.system() == 'Windows':
            shell_call('mingw32-make -j4')
        else:
            shell_call('make -j4')
        print('')

        print('BeeMake::Success!')
        return 0

class examplecommand(Command):
    def __init__(self, *args, **kwargs):
        super(examplecommand, self).__init__(
            name='build',
            help_short='Builds the project with its example project',
            *args, **kwargs)

    def execute(self, args, cwd):
        print('BeeMake::Building...')
        os.chdir(os.getcwd())
        if not os.path.exists('build/'):
             os.makedirs('build/')
        print('')
        os.chdir('build/')

        print('BeeMake::Running CMake...')
        if platform.system() == 'Windows':
            shell_call('cmake .. -G "MSYS Makefiles" -DBUILD_EXAMPLE="ON" -DCMAKE_BUILD_TYPE="Release"')
        else:
            shell_call('cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLE="ON" -DCMAKE_BUILD_TYPE="Release"')
        print('')

        print('BeeMake::Running make...')
        if platform.system() == 'Windows':
            shell_call('mingw32-make -j4')
        else:
            shell_call('make -j4')
        print('')

        print('BeeMake::Success!')
        return 0

def discovercommands():
    commands = {
	  'pull': pullcommand(),
	  'build': buildcommand(),
	  'example': examplecommand(),
	  'nuke': nukecommand(),
	}
    return commands

def runcommand(command, args, cwd):
    return command.execute(args, cwd)

def usage(commands):
    s = 'beemake.py command [--help]\n'
    s += '\n'
    s += 'Commands:\n'
    commandnames = commands.keys()
    commandnames.sort()

    for commandname in commandnames:
        s += '	%s\n' % (commandname)
    commandhelp = commands[commandname].help_short
    if commandhelp:
        s += '	%s\n' % (commandhelp)
    s += '\n'

    return s

def main():
    sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

    if sys.version_info < (2, 7):
        print('BeeMake::Error - Python 2.7+ required.')
        sys.exit(1)
        return

    commands = discovercommands()

    try:
        if len(sys.argv) < 2:
            raise ValueError('BeeMake::No command given')
        commandname = sys.argv[1]
        if not commandname in commands:
            raise ValueError('BeeMake::Command "%s" not found' % (commandname))

        command = commands[commandname]
        returncode = runcommand(command=command, args=sys.argv[2:], cwd=os.getcwd())

    except ValueError:
        print(usage(commands))
        returncode = 1
    except Exception as e:
        raise
        returncode = 1
        sys.exit(returncode)
    

if __name__ == '__main__':
    main()
