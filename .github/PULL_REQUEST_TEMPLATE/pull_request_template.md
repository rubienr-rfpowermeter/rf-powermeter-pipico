<!-- Please do not create a pull request without creating an issue first. -->
<!-- You can skip this if you're fixing a typo. -->

<!-- tick off passed steps by [x] -->
<!-- mark failed steps with   [!] -->
<!-- mark steps not run with  [--] -->

<!--
**Description**
TODO
-->
<!-- Please include a summary of the change and which issue(s) it fixes (use `closes #XXXX`) -->
<!-- Please include relevant motivation and context. -->
<!-- List any dependencies that are required for this change. -->

**Type of change**
<!-- Please delete options that are not applicable. -->

- enhancement cleanup or refactoring (non-breaking change)
- bug fix (non-breaking change which fixes an issue)
- new feature (non-breaking change which adds functionality)
- breaking change (fix or feature that would cause existing functionality to not work as expected)
- this change requires a documentation update

<!-- List old and new tests that verifies your changes or delete this section if not applicable. -->
**Tests to verify changes**

- Test A
- Test B (new test)
-

`ctest -R test.name`

- ...
- Test N

**Checklist**

- [] my code is auto-formatted and follows the style guidelines of this project
- [] I have performed a self-review of my own code
- [] I have commented my code in hard-to-understand areas and public interface
- [] I have made corresponding changes to the documentation
- [] my changes generate no new warnings
- [] I have added tests that prove my fix is effective or that my feature works
- [] new and existing unit tests pass locally with my changes
- [] I have checked my code and corrected any misspellings

**Testing**

- unit tests:
    - [] green in GitHub CI
    - [] green locally machine: `tests/unit/run.sh`

