# team33

GitHub Desktop workflow for the team, with the equivalent CLI command(s) listed under each step.

## Cloning the Repository

- Open GitHub Desktop.
- Select **File → Clone Repository**.
- Select the repository from the **GitHub.com** tab.
- Choose a local folder.
- Click **Clone**.

CLI equivalent:

```bash
git clone <url>
```

## Before Starting Work

Always pull the latest changes before making edits.

- Open GitHub Desktop.
- Select the repository.
- Click **Fetch Origin**.
- If updates are available, click **Pull Origin**.
- Check that you are working on the latest version of the project.

CLI equivalent:

```bash
git fetch
git pull
git status
```

## Creating a New Branch

Do not work directly on the `main` branch.

- Click **Current Branch**.
- Select **New Branch**.
- Name the branch using the format:
  - `feature/feature-name`
  - `bugfix/bug-description`
  - `research/topic-name`

CLI equivalent:

```bash
git branch            # list existing branches
git switch -c <new-branch>   # create and switch to the new branch
```

## Committing Changes

- Enter a short commit summary.
- Click **Commit to `branch-name`**.

CLI equivalent:

```bash
git add .
git commit -m "message"
```

## Pushing Changes

- Click **Push Origin**.
- Wait for the upload to complete.

CLI equivalent:

```bash
git push
```

## Creating a Pull Request

- Push your branch.
- Click **Create Pull Request**.
- Another team member should review the code before merging.

## Merging a Pull Request

- Review the Pull Request on GitHub.
- Confirm the changes are correct.
- Approve the Pull Request.
- Click **Merge Pull Request**.
- Delete the branch if it is no longer needed.

CLI equivalent (for merging locally instead of on GitHub):

```bash
git switch main          # check out the branch you're merging into
git merge <branch>        # merge the feature branch into it
git log --oneline         # confirm the merge
```
