    #!/usr/bin/env bash
set -euo pipefail

repo_url="https://github.com/w3c-cg/mnx.git"
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
vendor_dir="${repo_root}/third_party/w3c-mnx"
ref="${1:-main}"

tmp_dir="$(mktemp -d)"
cleanup() {
    rm -rf "${tmp_dir}"
}
trap cleanup EXIT

git clone "${repo_url}" "${tmp_dir}/mnx"
git -C "${tmp_dir}/mnx" checkout --detach "${ref}"
commit="$(git -C "${tmp_dir}/mnx" rev-parse HEAD)"

schema_src="${tmp_dir}/mnx/docs/mnx-schema.json"
examples_src="${tmp_dir}/mnx/docs/static/examples/json"
schema_dest="${vendor_dir}/mnx-schema.json"
schema_header_dest="${vendor_dir}/mnx-schema.xxd"

if [[ ! -f "${schema_src}" ]]; then
    echo "schema not found: ${schema_src}" >&2
    exit 1
fi

if [[ ! -d "${examples_src}" ]]; then
    echo "examples directory not found: ${examples_src}" >&2
    exit 1
fi

if ! command -v xxd >/dev/null 2>&1; then
    echo "xxd is required to generate ${schema_header_dest}" >&2
    exit 1
fi

mkdir -p "${vendor_dir}/examples"
find "${vendor_dir}/examples" -type f -name '*.json' -delete
cp "${schema_src}" "${schema_dest}"
xxd -i -n mnx_schema_json "${schema_dest}" > "${schema_header_dest}"
cp "${examples_src}"/*.json "${vendor_dir}/examples/"
printf '%s\n' "${commit}" > "${vendor_dir}/UPSTREAM_COMMIT"

for license_file in LICENSE LICENSE.md COPYING NOTICE; do
    rm -f "${vendor_dir}/${license_file}"
    if [[ -f "${tmp_dir}/mnx/${license_file}" ]]; then
        cp "${tmp_dir}/mnx/${license_file}" "${vendor_dir}/${license_file}"
    fi
done

echo "Updated third_party/w3c-mnx to ${commit}"
