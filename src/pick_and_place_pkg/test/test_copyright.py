#
#
#
# Unless required by applicable law or agreed to in writing, software
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

import pytest


@pytest.mark.linter
    rc = main(argv=['.', 'test'])
    assert rc == 0, 'Found errors'
