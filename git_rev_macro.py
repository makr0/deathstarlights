import subprocess

revision = (
    subprocess.check_output(["git", "rev-parse", "HEAD"])
    .strip()
    .decode("utf-8")
)
remoteurl = (
    subprocess.check_output(["git", "remote", "get-url", "origin"])
    .strip()
    .decode("utf-8")
)
print("'-DGIT_REV=\"%s\"\n -DGIT_URL=\"%s\"'" % (revision , remoteurl))